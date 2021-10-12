

# Trapmine::AMQPWriter
A Zeek plugin to send logs to RabbitMQ using AMQP.

* [Installation](#installation)
* [Parameters](#parameters)
* [Examples](#examples)

### Installation
To install the plugin, you need to build [zeek](https://docs.zeek.org/en/master/install.html) and [rabbitmq-c](https://github.com/alanxz/rabbitmq-c). 

```sh
git clone https://github.com/trapmine/zeek-amqp
cd zeek-amqp
./configure --zeek-dist=/path/to/zeek --with-rabbitmq-c=/path/to/rabbitmq-c
make
make install  
```

### Parameters
You can set up the following parameters for this plugin.
* LogAMQP::hostname
* LogAMQP::amqp_port
* LogAMQP::username
* LogAMQP::password
* LogAMQP::vhost
* LogAMQP::queue_name
* LogAMQP::exchange
* LogAMQP::routing_key

### Examples
You can simply redefine all the parameters in a zeek script to send all logs to RabbitMQ.

```
redef LogAMQP::hostname = "localhost";
redef LogAMQP::amqp_port = 5672;
redef LogAMQP::username = "guest";
redef LogAMQP::password = "guest";
redef LogAMQP::vhost = "/";
redef LogAMQP::queue_name = "test_queue";
redef LogAMQP::exchange = "";
redef LogAMQP::routing_key = "test_queue";
redef Log::default_writer = Log::WRITER_AMQP;
```

You can also attach a filter with different log streams to send them to separate queues.

```
event zeek_init() &priority=1 {
	local filter: Log::Filter = [
		$name="amqp_conn",
		$config=table(
			["hostname"] = "localhost",
			["amqp_port"] = "5672",
			["username"] = "guest",
			["password"] = "guest",
			["vhost"] = "/",
			["queue_name"] = "test_queue",
			["exchange"] = "",
			["routing_key"] = "test_queue"),
		$writer=Log::WRITER_AMQP
	];
	Log::add_filter(Conn::LOG, filter);
}
```
Note that the all of the parameters, including amqp_port, passed to the config parameter in Log::Filter are strings.

