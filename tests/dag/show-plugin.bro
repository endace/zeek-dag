# @TEST-EXEC: ${ZEEK_EXEC} -NN Endace::DAG |sed -e 's/version.*)/version)/g' >output
# @TEST-EXEC: btest-diff output
