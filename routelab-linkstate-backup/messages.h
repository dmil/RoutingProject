#ifndef _messages
#define _messages

#include <iostream>
#include "messages.h"

using std::ostream;

#if defined(GENERIC)
struct RoutingMessage {
 public:
  ostream & Print(ostream &os) const;
};
#endif

#if defined(LINKSTATE)
struct RoutingMessage {
	int source;
	int destination;
	int latency;
	int age;

  RoutingMessage();
  RoutingMessage(int a, int s, int d, int l);
  RoutingMessage(const RoutingMessage &rhs);
  RoutingMessage &operator=(const RoutingMessage &rhs);
  int GetSource() const;
  int GetDestination() const;
  int GetLatency() const;
  int GetAge() const;

  ostream & Print(ostream &os) const;
};
#endif

#if defined(DISTANCEVECTOR)
struct RoutingMessage {
  int source;
  int destination;
  int latency;

  RoutingMessage();
  RoutingMessage(int s, int d, int l);
  RoutingMessage(const RoutingMessage &rhs);
  RoutingMessage &operator=(const RoutingMessage &rhs);

  ostream & Print(ostream &os) const;
};
#endif

inline ostream & operator<<(ostream &os, const RoutingMessage &m) { return m.Print(os); }

#endif
