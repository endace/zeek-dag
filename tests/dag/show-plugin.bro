# @TEST-EXEC: bro -NN Endace::DAG |sed -e 's/version.*)/version)/g' >output
# @TEST-EXEC: btest-diff output
