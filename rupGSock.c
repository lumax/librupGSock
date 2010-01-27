/* 
 * File:   rupGSock.c
 * Author: Bastian Ruppert
 * Date: 27.01.2010
 */
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <stdio.h>
#include <glib.h>

//#include <defs.h>

#include "rupGSock.h"

static gboolean prepare  (GSource *source,gint *timeout_);
static gboolean check    (GSource *source);
static gboolean dispatch (GSource *source,GSourceFunc callback,gpointer user_data);

static GSourceFuncs rupGSockInFnks = {
    .prepare = prepare,
    .check = check,
    .dispatch = dispatch,
    .finalize = NULL,
  };

static void setFdEvents(rupGSock_t * src,\
			gboolean (*error)(GSource * src),	\
			gboolean (*hup)(GSource * src),  \
			void (*read)(GSource * src))
{
  //G_IO_OUT
  if(error)
    {
      src->error = error;
      src->fd.events |=G_IO_ERR;
    }
  if(hup)
    {
      src->hup = hup;
      src->fd.events |= G_IO_HUP;
    }
  if(read)
    {
      src->read = read;
      src->fd.events |= G_IO_IN;
    }
  g_source_add_poll((GSource *)src,&src->fd); 
}

GSource * rupGSockInitSTDIN(gboolean (*error)(GSource * src),\
			    gboolean (*hup)(GSource * src),\
			     void (*read)(GSource * src))
{
  rupGSock_t * ret =(rupGSock_t*)g_source_new(&rupGSockInFnks,\
					      sizeof(rupGSock_t));
  ret->fd.fd = 0;
  ret->timeout = -1;
  setFdEvents(ret,error,hup,read);
  return (GSource *)ret;
}

GSource * rupGSockInitSock(char * socketname,			\
			   gboolean (*error)(GSource * src),	\
			   gboolean (*hup)(GSource * src),		\
			   void (*read)(GSource * src))
{
  rupGSock_t * ret =(rupGSock_t*)g_source_new(&rupGSockInFnks,		\
					      sizeof(rupGSock_t));
  strcpy(ret->sa.sun_path, socketname);
  ret->sa.sun_family = AF_UNIX;
  ret->fd.fd = socket(AF_UNIX, SOCK_STREAM, 0);

  if(ret->fd.fd==-1) 
    {
      goto _error;
    }
  
  if(connect(ret->fd.fd,			\
	     (struct sockaddr *)&ret->sa,	\
	     sizeof(ret->sa) ) )
    {
      goto _error;
    }
	
     ret->timeout = -1;
     setFdEvents(ret,error,hup,read);
     return (GSource *)ret;
  _error:
     g_source_unref((GSource *)ret);
     return NULL;
     
}

static gboolean prepare(GSource *source,gint *timeout_)
{
  rupGSock_t * sock = (rupGSock_t*)source;
  *timeout_ = sock->timeout;
  return FALSE;
}

static gboolean check(GSource *source)
{
  /*
    POLLOUT 
    Writing now will not block.
    POLLWRNORM 
    Equivalent to POLLOUT.
    POLLWRBAND 
    Priority data may be written.
    Linux also knows about, but does not use POLLMSG. 
  */ 
  rupGSock_t * sock = (rupGSock_t*)source;
  printf("check ");
 if(sock->fd.revents & POLLNVAL) 
    {
      /*
	POLLNVAL 
	Invalid request: fd not open (output only).
	When compiling with _XOPEN_SOURCE defined, one also has 
	the following, which convey no further information beyond 
	the bits listed above:
       */
      printf("errno = EINVAL;\n");
      return TRUE;
    }
  if(sock->fd.revents & POLLERR) 
    {
      /*
	POLLERR 
	Error condition (output only).
      */
      printf("errno = EIO;\n");
      return TRUE;
    }
  if(sock->fd.revents & POLLRDHUP) 
    {
      /*
	POLLRDHUP (since Linux 2.6.17) 
	Stream socket peer closed connection, or shut down writing 
	half of connection. The _GNU_SOURCE feature test macro must 
	be defined in order to obtain this definition.
       */
      printf("POLLRDHUP\n");
      return TRUE;
    }	
  if(sock->fd.revents & POLLHUP)
    {
      /*
	POLLHUP 
	Hang up (output only).
      */
      printf("POLLHUP\n");
      return TRUE;
    }
  if(sock->fd.revents & (POLLIN | POLLPRI | POLLRDNORM))
    {
      /*
	POLLIN
	There is data to read. 
	POLLPRI 
	There is urgent data to read (e.g., out-of-band data on TCP 
	socket; pseudo-terminal master in packet mode has seen state 
	change in slave).
	POLLRDNORM 
	Equivalent to POLLIN.
      */
      printf(" POLLIN | POLLPRI\n");
      return TRUE;
    }
  printf("FALSE\n");
  return FALSE;
}

static gboolean dispatch(GSource *source,\
			 GSourceFunc callback,\
			 gpointer user_data)
{
  rupGSock_t * sock = (rupGSock_t*)source;
  printf("dispatch");
  if(sock->fd.revents & POLLNVAL) 
    {
      printf("errno = EINVAL;\n");
      if(sock->error)
	{
	  (*sock->error)(source);
	}  
      return TRUE;
    }
  if(sock->fd.revents & POLLERR) 
    {
      printf("errno = EIO;\n");
      if(sock->error)
	{
	  (*sock->error)(source);
	}     
      return TRUE;
    }	
  if(sock->fd.revents & (POLLRDHUP|POLLHUP) )
    {
      printf("POLLRDHUP|POLLHUP\n");
      if(sock->hup)
	{
	  return (*sock->hup)(source);
	}
      return FALSE;
    }
  if(sock->fd.revents & (POLLIN | POLLPRI | POLLRDNORM))
    {
      printf(" POLLIN | POLLPRI | POLLRDNORM\n");
      if(sock->read)
	{
	  (*sock->read)(source);
	}
      return TRUE;
    }
  printf("\n");
  return TRUE;
  //return FALSE;???
}
