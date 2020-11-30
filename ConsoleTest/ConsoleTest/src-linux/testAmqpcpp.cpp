/*
 * testAmqpcpp.cpp
 *
 *  Created on: May 22, 2019
 *      Author: gyd
 */

#include "AMQPcpp.h"

using namespace std;

int i = 0;
int onCancel(AMQPMessage * message ) {
    cout << "cancel tag="<< message->getDeliveryTag() << endl;
    return 0;
}
int  onMessage( AMQPMessage * message  ) {
    uint32_t j = 0;
    char * data = message->getMessage(&j);
    if (data)
          cout << data << endl;

    i++;

    cout << "#" << i << " tag="<< message->getDeliveryTag() << " content-type:"<< message->getHeader("Content-type") ;
    cout << " encoding:"<< message->getHeader("Content-encoding")<< " mode="<<message->getHeader("Delivery-mode")<<endl;

    if (i > 10) {
        AMQPQueue * q = message->getQueue();
        q->Cancel( message->getConsumerTag() );
    }
    return 0;
};
int testAmqpcpp()
{
    try {
//      AMQP amqp;
//      AMQP amqp(AMQPDEBUG);

        AMQP amqp("topo:topo@10.1.24.141:5672");     // all connect string

        AMQPExchange * ex = amqp.createExchange("test");
        ex->Declare("test", "topic");

        AMQPQueue * qu1 = amqp.createQueue("q1");
        qu1->Declare();
        qu1->Bind( "e", "");

        string ss = "message 1 ";

        ex->setHeader("Delivery-mode", 2);
        ex->setHeader("Content-type", "text/text");
        ex->setHeader("Content-encoding", "UTF-8");

        ex->Publish(  ss , ""); // publish very long message
        // receive
        AMQPQueue * qu2 = amqp.createQueue("q2");

        qu2->Declare();
        qu2->Bind( "e", "");

        qu2->setConsumerTag("tag_123");
        qu2->addEvent(AMQP_MESSAGE, onMessage );
        qu2->addEvent(AMQP_CANCEL, onCancel );

        qu2->Consume(AMQP_NOACK);//



    } catch (AMQPException &e)
    {
        std::cout << e.getMessage() << std::endl;
    }

    return 0;

}


