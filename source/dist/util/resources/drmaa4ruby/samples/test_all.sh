#!/bin/sh
./example.rb && ./array.rb && ./sig.rb /bin/sleep 1 && ./stream_and_trap.rb && ./threads.rb && ./attributes.rb
ret=$?
if [ $ret -ne 0 ]; then
	echo "##### failed #####"
else
	echo "##### success #####"
fi
exit $ret
