#include "messages.h"

using namespace std;

#if defined(GENERIC)

ostream &RoutingMessage::Print(ostream &os) const
{ 
	os << "RoutingMessage()" << endl; 
	return os; 
}

#endif

#if defined(LINKSTATE)

RoutingMessage::RoutingMessage(){}
RoutingMessage::RoutingMessage(const RoutingMessage &rhs) {
	age = rhs.age;
	source = rhs.source;
	destination = rhs.destination;
	latency = rhs.latency;
}
RoutingMessage::RoutingMessage(int a, int s, int d, int l) {
	age = a;
	source = s;
	destination = d;
	latency = l;
}

ostream &RoutingMessage::Print(ostream &os) const
{ 
	os << "New path from " << source << " to " << destination << " of " << latency << " with age " << age << endl; 
	return os; 
}

int 
RoutingMessage::GetAge() const
{ 
	return age; 
}

int 
RoutingMessage::GetSource() const
{ 
	return source; 
}

int
RoutingMessage::GetDestination() const
{ 
	return destination; 
}

int 
RoutingMessage::GetLatency() const 
{ 
	return latency; 
}


#endif

/* DISTANCE VECTOR */

#if defined(DISTANCEVECTOR)

RoutingMessage::RoutingMessage() {}

RoutingMessage::RoutingMessage(int s, int d, int l)
{
	source = s;
	destination = d;
	latency = l;
}

RoutingMessage::RoutingMessage(const RoutingMessage &rhs)
{
	source = rhs.source;
	destination = rhs.destination;
	latency = rhs.latency;
}

ostream &RoutingMessage::Print(ostream &os) const
{ 
	os << "path from " << source << " to " << destination << " of " << latency << endl; 
	return os; 
}

int 
RoutingMessage::GetSource() const
{ 
	return source; 
}

int
RoutingMessage::GetDest() const
{ 
	return destination; 
}

int 
RoutingMessage::GetLatency() const 
{ 
	return latency; 
}


#endif

