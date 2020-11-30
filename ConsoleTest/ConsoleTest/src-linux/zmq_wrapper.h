/*
wrapper zeroMQ PUB/SUB 

notice
for remote connection, 
 	server should not listen on 127.0.0.1 !!!
  	use real-ip-address instead
refer
	http://zguide.zeromq.org/page:all
changelog
	2016-09-05 v1.0
	2016-09-12 v1.01 add heartbeat
	2016-11-23 v1.02 support udp
*/

#ifndef ZMQ_WRAPPER_H
#define ZMQ_WRAPPER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>

#include "zmq.h"
#include <inttypes.h>

#ifndef u32
typedef unsigned int u32;
#endif

enum
{
	EZMQ_TYPE_PUBSUB	= 1,
		//����-����ģʽ����һ�Զ�ģʽ��
		//��һ�������߷�������Ϣ���ȳ��ķ�ʽ���������߷ַ�
	EZMQ_TYPE_REQREP	= 2	
		//����-�ָ�ģʽ������ZMQ_REQ�ͻ�����һ������ZMQ_REP����˷�������
		//���ҽ���������Ķ�ÿһ�����Ͷ˵Ļָ���
};
enum 
{ 
	EZMQ_RECV_BADLEN	=-5,
	EZMQ_RECV_TRUNCATED	=-4,
	EZMQ_RECV_HEARTBEAT	=-3,
	EZMQ_RECV_BUSY		=-2,
	EZMQ_RECV_FAIL		=-1,
	EZMQ_RECV_NODATA	=0
						//>0: has data
};


#define ZMQ_WRAPPER_FIX_STR 		"zmq"
#define ZMQ_WRAPPER_HEARTBEAT_STR 	"heartbeat"
#define ZMQ_WRAPPER_HEARTBEAT_LEN	9

#if 1
#define ZMQ_WRAPPER_MAXMSGLEN		0xff00		//65280
#define ZMQ_WRAPPER_MAXTOTLEN		0x10000		//65536
#else
#define ZMQ_WRAPPER_MAXMSGLEN		0xfff00		//1044480
#define ZMQ_WRAPPER_MAXTOTLEN		0x100000	//1M
#endif

#define ZMQ_WRAPPER_TIMEOUT			1			//ms
#define ZMQ_WRAPPER_MAXBUF			0x1000000	//16M
#define ZMQ_WRAPPER_MAXHWM			30000		//zmq default: 1000 is too small, 
												//if traffic is 10k/s,then HWM is at least 10k
												//32M/0.3~1K=32~100k
#define ZMQ_WRAPPER_MAXBACKLOG		ZMQ_WRAPPER_MAXHWM
#define ZMQ_WRAPPER_MAXDEX			999999999
#define ZMQ_WRAPPER_LEN				12 //4+4+4

struct t_zmq_wrapper
{
	char	fix[4];	//4: ZMQ_WRAPPER_FIX_STR
	int		len;	//4: max:ZMQ_WRAPPER_MAXMSGLEN
	u32		dex; 	//4: max:ZMQ_WRAPPER_MAXDEX
	char	buf[ZMQ_WRAPPER_MAXTOTLEN];

	t_zmq_wrapper() : len(0),dex(0)
	{ }
	t_zmq_wrapper(int) : len(0),dex(0)
	{
		strcpy(fix, ZMQ_WRAPPER_FIX_STR);
		memset(buf, 0, 	ZMQ_WRAPPER_MAXTOTLEN);
	}
	
	t_zmq_wrapper(const void* str,int inlen,int index)
	{
		strcpy(fix, ZMQ_WRAPPER_FIX_STR);
		len=inlen;
		dex=index;
		memcpy(buf, str, len);
	}
	void set(const void* str,int inlen,int index)
	{
		len=inlen;
		dex=index;
		memcpy(buf, str, len);			
	}

	int check(const char* inbuf, int inlen,bool debug)
	{
		//begin with fix
		const char* q=inbuf;
		if(0!=strncmp(q, ZMQ_WRAPPER_FIX_STR, 3))
		{
			if(debug) printf("not fix: str\n");
			return EZMQ_RECV_TRUNCATED;
		}
		q+=4;

		//len
		len=*(int*)q;
		if(len != inlen-ZMQ_WRAPPER_LEN)
		{
			if(debug) printf("wrong len: buildin=%d recv=%d\n", len, inlen-ZMQ_WRAPPER_LEN);
			return EZMQ_RECV_BADLEN;
		}
		q+=4;
		
		//dex
		dex=*(u32*)q;
		q+=4;

		return inlen-ZMQ_WRAPPER_LEN;
	}
};


template<int T=EZMQ_TYPE_PUBSUB>
class cZmqWrapperCom
{
public:
		
private:
	enum { EZMQ_SUB_MAXLEN = 6 };
	
	char		proto[16];		//tcp or udp
	bool		debug;			//
	int			sublen;			//for send
	char		substr[EZMQ_SUB_MAXLEN+1];
	char		cs;				//Client or Server
	int			socktype;		//pub/sub
	char		subtype[64];	//subscribe-type

	void* 		cntx;			//zmq.context
	void*		sock;			//zmq.socket
	char		addr[256];		//

	uint64_t	spkg;			//for stat send-ok
	uint64_t	sbyte;			//
	uint64_t	fpkg;			//for stat send-fail
	uint64_t	fbyte;			//
	uint64_t	rpkg;			//for stat recv
	uint64_t	rbyte;			//
	
	
	u32			sdex;			//for header
	u32			rdex;			//
	int			rlen;			//
	char*		rstr;			//
	uint64_t	hb_send;		//for hearbeat
	uint64_t	hb_recv;
	
	void zmq_first_init()
	{
		debug=0;
		sublen=1;
		cs=0;
		socktype=-1;
		cntx=0;
		sock=0;
	    spkg=sbyte=0LL;
	    fpkg=fbyte=0LL;
	    rpkg=rbyte=0LL;
	    sdex=rdex=0;
	    rlen=0;
	    rstr=0;
	    hb_send=hb_recv=0;
		subtype[0]=0;
		strcpy(proto,"tcp");
	}
	
public:
	cZmqWrapperCom() 				{ zmq_first_init(); }
	~cZmqWrapperCom()				{ clear(); }
	void clear()
	{
   		if(sock) { zmq_close(sock); sock=0; }
   		if(cntx) { zmq_term (cntx); cntx=0; }
	}
	
	//---------------------------------------------------------------
	//set-config
	void set_debug(bool x)			{ debug=x; }
	void set_proto(const char* s)	{ if(s && *s) strcpy(proto,s); }
	void set_sublen(int x)			{ if(x>0) sublen=x; if(sublen>EZMQ_SUB_MAXLEN) sublen=EZMQ_SUB_MAXLEN; }

	//set-option
	//hwm: high water mark
	template <class X>
	int zmq_set_opt(int t,X x)
	{
		if(!sock) return 0;
		int rc=zmq_setsockopt(sock, t, &x, sizeof(x));
		if(0!=rc) { die("zmq_setsockopt(%d)",t); return 0; }
		return 1;
	}	
	int set_rcvhwm(int x)			{ return zmq_set_opt(ZMQ_RCVHWM,x); }
	int set_sndhwm(int x)			{ return zmq_set_opt(ZMQ_SNDHWM,x); }
	int set_backlog(int x)			{ return zmq_set_opt(ZMQ_BACKLOG,x); }
	int set_rcvbuf(int x)			{ return zmq_set_opt(ZMQ_RCVBUF,x); }
	int set_sndbuf(int x)			{ return zmq_set_opt(ZMQ_SNDBUF,x); }
	int set_maxmsgsize(int x)		{ return zmq_set_opt(ZMQ_MAXMSGSIZE,x); }
	int set_rcvtimeo(int x)			{ return zmq_set_opt(ZMQ_RCVTIMEO,x); }
	int set_sndtimeo(int x)			{ return zmq_set_opt(ZMQ_SNDTIMEO,x); }
	int set_linger(int x)			{ return zmq_set_opt(ZMQ_LINGER,x); }
	int set_keepalive(int x)		{ return zmq_set_opt(ZMQ_TCP_KEEPALIVE,x); }

	//---------------------------------------------------------------
	//get-config
	bool get_debug() const			{ return debug; }
	int  get_sublen() const			{ return sublen; }
	const char* get_addr() const	{ return addr; }
	const char* get_proto() const	{ return proto; }
	
	//get-stat
	uint64_t get_pkg() const		{ return spkg; }
	uint64_t get_byte() const		{ return sbyte; }
	uint64_t get_fail_pkg() const	{ return fpkg; }
	uint64_t get_fail_byte() const	{ return fbyte; }
	uint64_t get_recv_pkg() const	{ return rpkg; }
	uint64_t get_recv_byte() const	{ return rbyte; }
	uint64_t get_hb_send() const	{ return hb_send; }
	uint64_t get_hb_recv() const	{ return hb_recv; }

	//---------------------------------------------------------------
	//as server
	int server(int port)
	{
		return server("127.0.0.1",port);
	}
	int server(const char* ip,int port)
	{
		char srv[256];
		if(ip && *ip) sprintf(srv, "%s://%s:%d",        proto,ip,port);
		else          sprintf(srv, "%s://127.0.0.1:%d", proto,port);
		return server(srv);
	}
	int server(const char* srv)
	{
		if(strchr(srv,':')) {
			if(strstr(srv,"://")) strcpy(addr,srv);
			else                  sprintf(addr,"%s://%s", proto,srv);
		}else{
			sprintf(addr, "%s://127.0.0.1:%s", proto,srv);
		}
		cs='s';
		switch(T)
		{
		case EZMQ_TYPE_PUBSUB: socktype=ZMQ_PUB; break;
		case EZMQ_TYPE_REQREP: socktype=ZMQ_REP; break;
		}

	   	cntx = zmq_init (1);
	    if(!cntx) { die("zmq_init"); return 0; }
	
	    sock = zmq_socket (cntx, socktype);
	   	if(!sock) { die("zmq_socket"); return 0; }

		//zmq_unbind(sock, addr);
    	int rc = zmq_bind (sock, addr);
    	if(0!=rc) { die("zmq_bind"); return 0; }

		return after_socket();
	}

	//---------------------------------------------------------------
	//as Client
	int client(int port,const char* msgtype="")
	{
		char str[256];
		sprintf(str, "%s://127.0.0.1:%d", proto,port);
		return client(str,msgtype);
	}
	int client(const char* ip,int port,const char* msgtype="")
	{
		char srv[256];
		if(ip && *ip) sprintf(srv, "%s://%s:%d",       proto,ip,port);
		else          sprintf(srv, "%s://127.0.0.1:%d",proto,port);
		return client(srv,msgtype);
	}
	int client(const char* srv,const char* msgtype="")
	{
		if(strchr(srv,':')) {
			if(strstr(srv,"://")) strcpy(addr,srv);
			else                  sprintf(addr,"%s://%s", proto,srv);
		}else{
			sprintf(addr, "%s://127.0.0.1:%s", proto,srv);
		}
		cs='c';
		strcpy(subtype, msgtype);
		switch(T)
		{
		case EZMQ_TYPE_PUBSUB: socktype=ZMQ_SUB; break;
		case EZMQ_TYPE_REQREP: socktype=ZMQ_REQ; break;
		}

	   	cntx = zmq_init (1);
	    if(!cntx) { die("zmq_init"); return 0; }

	    sock = zmq_socket (cntx, socktype);
	   	if(!sock) { die("zmq_socket"); return 0; }

		//zmq_disconnect(sock, addr);
	    int rc = zmq_connect (sock, addr);
	    if(0!=rc) { die("zmq_connect"); return 0; }

		return after_socket();
	}


	//---------------------------------------------------------------
	//send
	int send_heartbeat()
	{
		if(ZMQ_SUB!=socktype) return 0;
		int rc=send(ZMQ_WRAPPER_HEARTBEAT_STR, ZMQ_WRAPPER_HEARTBEAT_LEN);
		if(rc) hb_send++;
		return rc;
	}
	int send_subtype(const char* type)
	{
		int rc=0;
	
		//send first: sub-type
		//if-no-type: a      (sublen byte)
		//else:       substr (sublen byte)
		const char* p=(type && type[0]) ? type : "a";
		snprintf(substr, sublen, p);
		substr[sublen]=0;

		rc=zmq_send (sock, substr, sublen, ZMQ_SNDMORE);
		if(-1==(size_t)rc)
		{
			die("zmq_send type=%s", (type?type:""));
			return 0;
		}
		return 1;
	}
	//for string, send the end '\0' also
	int send(const char* str,const char* type=NULL)
	{
		if(!str || 0==str[0]) return 0;
		int len=strlen(str);
		return send(str, len+1, type);	
	}
	int send(const void* str,int len,const char* type=NULL)
	{
		if(!str || len<1) return 0;

		//for PUB/SUB
		//[1] must have type firstly
		if(ZMQ_PUB==socktype)
		{
			if(! send_subtype(type) ) 
			{
				add_fail(len); 
				die("zmq_send fail subtype: str=%p len=%d", str,len);
				return 0; 
			}
		}
		
		//[2] send content
		//begin with fix:3,
		//follow by length:5,then dex:8
		++sdex;
		if(sdex > ZMQ_WRAPPER_MAXDEX) sdex=1;

		static t_zmq_wrapper t(1);
		t.set(str,len,sdex);

		int tlen=len+ZMQ_WRAPPER_LEN;
	    int rc=zmq_send (sock, &t, tlen, 0);
	    if(-1==(size_t)rc)
	    {
	    	add_fail(len);	    	
			die("zmq_send return -1: str=%p len=%d", str,len);
			return 0;
	    }
	    if(rc != tlen)
	    {
	    	add_fail(len);
	    	die("zmq_send truncated: str=%p len=%d rc=%d", str,tlen,rc);
	    	return 0;
	    }

	    add_send(len);
	    if(debug) printf("send ok: %llu %llu %d\n", spkg,sbyte,len);
	    
	    return rc;
	}

	//---------------------------------------------------------------
	//recv
	//input  (1)str: save content to str
	//       (2)len: if len is smaller than real-data, truncate it
	//return (1)-3:heart-beat -2:busy -1:fail 0:no-data >0:data-len
	//       (2)pdex: the index
	int recv(void* str,int len,int* pdex=NULL)
	{
		//the buffer is too long, so use static
		static char rbuf[ZMQ_WRAPPER_MAXTOTLEN]={0};
		
		int  rc = zmq_recv (sock, 
							rbuf, 
							ZMQ_WRAPPER_MAXTOTLEN, 
							(ZMQ_WRAPPER_TIMEOUT ? 0 : ZMQ_DONTWAIT)
							);
		if(debug) printf("recv=%d\n", rc);
		if(0==rc || -1==(size_t)rc)
		{
			switch(errno)
			{
			case EAGAIN:
			case EINTR:  return EZMQ_RECV_BUSY; break;
			default:     die("zmq_recv"); 
						 return EZMQ_RECV_FAIL; break;
			}
		}else{
			switch(socktype)
			{
			case ZMQ_SUB:
				//ignore sub.type
				if(rc <= sublen+ZMQ_WRAPPER_LEN)
				{
					if(debug) printf("not data\n");
					return EZMQ_RECV_NODATA;
				}
				break;
			default:
				if(rc <= ZMQ_WRAPPER_LEN)
				{
					if(debug) printf("not data\n");
					return EZMQ_RECV_NODATA;
				}
				break;
			}
	
			static t_zmq_wrapper t;
			int reason=t.check(rbuf, rc, debug);
			if(reason < 1) 
			{
				if(debug) printf("check failed, reason=%d\n", reason);
				return reason;
			}

			//ok, recv-dex/len/str
			if(pdex) *pdex=t.dex;
			rdex=t.dex;
			rlen=rc  -ZMQ_WRAPPER_LEN;
			rstr=rbuf+ZMQ_WRAPPER_LEN;

			//is heart-beat
			if(0==strncmp(rstr, ZMQ_WRAPPER_HEARTBEAT_STR, ZMQ_WRAPPER_HEARTBEAT_LEN))
			{
				hb_recv++;
				if(debug) printf("it is heart-beat\n");
				return EZMQ_RECV_HEARTBEAT;
			}

			//ok:
			int nreal=rlen;
			if(nreal > len)
			{
				if(debug) printf("warn: recv len is too short (recv=%d input=%d)\n", nreal, len);
				nreal=len;
			}
			memcpy(str, rstr, nreal);
			//test says that following has no improvement:
			//strncpy((char*)str, (const char*)rstr, nreal);

			add_recv(nreal);
			if(debug) printf("recv ok: recv.pkg=%" PRIu64 "byte=%" PRIu64 " cur.len=%d save.len=%d\n", rpkg,rbyte,rlen,nreal);
	
			return nreal;
		}
	}

private:
	//add
	void add_send(int x)			{ spkg++; sbyte+=x; }
	void add_fail(int x)			{ fpkg++; fbyte+=x; }
	void add_recv(int x)			{ rpkg++; rbyte+=x; }

	void die(const char* fmt,...)
	{
		if(debug)
		{
			int x=zmq_errno();
			char str[BUFSIZ];
			va_list ap;
			va_start(ap, fmt);
			vsprintf(str,fmt, ap);
			va_end (ap);			
			printf("%s: error=%d (%s)\n", str, x, zmq_strerror(x));
		}
	}

	int after_socket()
	{
		int rc=0;
		if(ZMQ_SUB==socktype)
		{
		    //  Subscribe for all messages.
		    rc = zmq_setsockopt (sock, ZMQ_SUBSCRIBE, subtype, strlen(subtype));
		    if(0!=rc) { die("zmq_setsockopt(ZMQ_SUBSCRIBE)"); return 0; }
		}
		//unit: milliseconds
		set_rcvtimeo(ZMQ_WRAPPER_TIMEOUT);
		set_sndtimeo(ZMQ_WRAPPER_TIMEOUT);
		set_maxmsgsize(ZMQ_WRAPPER_MAXMSGLEN);
		set_rcvbuf(ZMQ_WRAPPER_MAXBUF); //A value of zero means leave the OS default unchanged
		set_sndbuf(ZMQ_WRAPPER_MAXBUF);
		set_rcvhwm(ZMQ_WRAPPER_MAXHWM);
		set_sndhwm(ZMQ_WRAPPER_MAXHWM);
		set_backlog(ZMQ_WRAPPER_MAXBACKLOG);
		set_keepalive(1);
		return 1;
	}
		
	//---------------------------------------------------------------
	//msg
	//!!! not used !!!
	int recv_with_msg(void* str,int len)
	{
		int n=0;
		while(1)
		{
			//Create an empty 0MQ message to hold the message part
		    zmq_msg_t msg;
		    int rc = zmq_msg_init (&msg);
		    if(0!=rc) { die("zmq_msg_init"); break; }

		    //Block until a message is available to be received from socket
		    rc = zmq_msg_recv (&msg, sock, 0);
		    if(-1==rc) 
		    {
				switch(errno)
				{
				case EAGAIN:
				case EINTR:  rc=0; break;
				default:     die("zmq_msg_recv"); break;
				}
		    }
		    if(rc<1) { break; }
		    int x=rc; //msg length

		    //Determine if more message parts are to follow
			int64_t more=0LL;
			size_t  more_size = sizeof (more); 
		    rc = zmq_getsockopt (sock, ZMQ_RCVMORE, &more, &more_size);
		    if(0!=rc)  { die("zmq_getsockopt(ZMQ_RCVMORE)"); break; }

		    if(!more) 
		    {
	            char* buffer = (char*)zmq_msg_data(&msg);
	            if(buffer && x>0)
	            {
				   	memcpy(str, buffer, x);
				   	n=x;
				   	add_recv(x);
					//printf("recv %llu %llu %d\n", rpkg,rbyte,x);
				}
		    }else{
		    	//it is sub-flag, ignore it	
		    }
		    zmq_msg_close (&msg);

		    if(!more)
		    {
		    	break;
		    }
	    }

		return n;
	}	
	
	
};

typedef cZmqWrapperCom<EZMQ_TYPE_PUBSUB> cZmqWrapper;
typedef cZmqWrapperCom<EZMQ_TYPE_REQREP> cZmqRR;


#endif
