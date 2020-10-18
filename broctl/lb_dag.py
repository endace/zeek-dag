# See the file "COPYING" in the main distribution directory for copyright.

# This plugin supports Endace DAG card load balancing across multiple consecutive (even) receive streams.
# Does not currently configure DAG card for steering. See README for useage instructions.
# Supports broctl.cfg PFRINGFirstAppInstance to specify starting stream number.

try:
    from ZeekControl import plugin
    from ZeekControl import config
except ImportError:
    from BroControl import plugin
    from BroControl import config

import re

class LBDAG(plugin.Plugin):
    def __init__(self):
        super(LBDAG, self).__init__(apiversion=1)

    def name(self):
        return "dag"

    def pluginVersion(self):
        return 1

    def init(self):
        useplugin = False
        first_app_instance = int(config.Config.pfringfirstappinstance)
        host = None
        last_interface = None
        last_dag_lb_method = None
        dag_lb_method = None
        for nn in self.nodes():
            if nn.type != "worker" or not nn.interface.startswith("endace::") or not nn.lb_procs or nn.lb_method != "custom":
                continue

            useplugin = True

            dag_lb_method = nn.dag_lb_method if nn.dag_lb_method else "stream"

            # Reset for different hosts or if the user-specified card or stream number changes
            if nn.host != host or nn.interface != last_interface or dag_lb_method != last_dag_lb_method:
                host = nn.host
                last_interface = nn.interface
                last_dag_lb_method = dag_lb_method

                match = re.match("endace::(.*?)([0-9]*)(?:\:([0-9]*))?$", nn.interface)
                dag_card_name, dag_card_num, dag_stream_num = match.groups()
                card_name = dag_card_name if dag_card_name is not None else "dag"

                if dag_card_num is not None:
                    card_num = int(dag_card_num)
                elif dag_lb_method == "card":
                    card_num = first_app_instance
                else:
                    card_num = 0

                if dag_stream_num is not None:
                    stream_num = int(dag_stream_num)
                elif dag_lb_method == "stream":
                    stream_num = first_app_instance
                else:
                    stream_num = None

            if stream_num is not None:
                nn.interface = "endace::%s%d:%d" % (card_name, card_num, stream_num)
            else:
                nn.interface = "endace::%s%d" % (card_name, card_num)

            if dag_lb_method == "stream":
                # For the case where a user is running dag HLB
                stream_num += 2
            elif dag_lb_method == "card":
                # Multiple dag cards each with one stream
                card_num += 1

        return useplugin

    # TODO: do we want to add these as options as well?
    def nodeKeys(self):
        return ["lb_method"]