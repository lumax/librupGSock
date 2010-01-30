/* 
 * File:   rupGSock.h
 * Author: Bastian Ruppert
 * Date: 27.01.2010
 */
#ifndef __rupGSock_h__
#define __rupGSock_h__

typedef struct
{
  GSource parent;
  GPollFD fd;
  struct sockaddr_un sa;
  gint timeout;
  int verbose;
  gboolean (*error)(GSource * src);
  gboolean (*hup)(GSource * src);
  void (*read)(GSource * src);
}rupGSock_t;

GSource * rupGSockInitSTDIN(gboolean (*error)(GSource * src),\
			    gboolean (*hup)(GSource * src),\
			    void (*read)(GSource * src));

GSource * rupGSockInitSock(char * socket,			\
			   gboolean (*error)(GSource * src),	\
			   gboolean (*hup)(GSource * src),		\
			   void (*read)(GSource * src));
#endif
