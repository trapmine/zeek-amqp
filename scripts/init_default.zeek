module LogAMQP;

export {
	const hostname = "localhost" &redef;
	const amqp_port = 5672 &redef;
	const vhost = "/" &redef;
	const username = "guest" &redef;
	const password = "guest" &redef;
	const queue_name = "test_queue" &redef;
	const exchange = "" &redef;
	const routing_key = "test_queue" &redef;
}
