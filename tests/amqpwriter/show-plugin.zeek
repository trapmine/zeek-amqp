# @TEST-EXEC: zeek -NN Trapmine::AMQPWriter |sed -e 's/version.*)/version)/g' >output
# @TEST-EXEC: btest-diff output
