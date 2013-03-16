#include "node.h"
#include "context.h"
#include "error.h"
#define MAX 2147483647

Node::Node(const unsigned n, SimulationContext *c, double b, double l) : 
    number(n), context(c), bw(b), lat(l) 
{
  #if defined(DISTANCEVECTOR)
    if(c != NULL) {
      latencyTable = new Table(*this);
    }
  #endif

  #if defined(LINKSTATE)
    if (c != NULL) {
      linkTable = new Table();
    }
  #endif
}

Node::Node() 
{ throw GeneralException(); }

Node & Node::operator=(const Node &rhs) 
{
  return *(new(this)Node(rhs));
}

void Node::SetNumber(const unsigned n) 
{ number=n;}

unsigned Node::GetNumber() const 
{ return number;}

void Node::SetLatency(const double l)
{ lat=l;}

double Node::GetLatency() const 
{ return lat;}

void Node::SetBW(const double b)
{ bw=b;}

double Node::GetBW() const 
{ return bw;}

Node::~Node()
{}

// Implement these functions  to post an event to the event queue in the event simulator
// so that the corresponding node can recieve the ROUTING_MESSAGE_ARRIVAL event at the proper time
void Node::SendToNeighbors(const RoutingMessage *m)
{
  deque<Node*> * neighbors = GetNeighbors();
  //iterate through neighbors and send each the routing message
  for(deque<Node*>:: iterator itr = neighbors->begin(); itr != neighbors->end(); ++itr){
    //const RoutingMessage * l = new RoutingMessage(*m);
    //SendToNeighbor(*itr,l);
    SendToNeighbor(*itr,m);
  }
}

void Node::SendToNeighbor(const Node *n, const RoutingMessage *m)
{
  //create new link to pass to routing message
  const Link * rmLink = new Link(number, n->number, context, 0, 0);//maybe context should be replaced with NULL
  Link * matchedLink = context->FindMatchingLink(rmLink);

  if(matchedLink != 0) {
   Event * event = new Event(context->GetTime()+matchedLink->GetLatency(),ROUTING_MESSAGE_ARRIVAL, new Node(*n), new RoutingMessage(*m));
   //can change shit around here
   context->PostEvent(event);
  }

  delete rmLink;
}

deque<Node*> *Node::GetNeighbors()
{ return context->GetNeighbors(this); }

void Node::SetTimeOut(const double timefromnow)
{
  context->TimeOut(this,timefromnow);
}

bool Node::Matches(const Node &rhs) const
{
  return number==rhs.number;
}


#if defined(GENERIC)
void Node::LinkHasBeenUpdated(const Link *l)
{
  cerr << *this << " got a link update: "<<*l<<endl;
  //Do Something generic:
  SendToNeighbors(new RoutingMessage);
}


void Node::ProcessIncomingRoutingMessage(const RoutingMessage *m)
{
  cerr << *this << " got a routing messagee: "<<*m<<" Ignored "<<endl;
}


void Node::TimeOut()
{
  cerr << *this << " got a timeout: ignored"<<endl;
}

Node *Node::GetNextHop(const Node *destination) const
{
  return 0;
}

Table *Node::GetRoutingTable() const
{
  return new Table;
}


ostream & Node::Print(ostream &os) const
{
  os << "Node(number="<<number<<", lat="<<lat<<", bw="<<bw<<")";
  return os;
}

#endif

#if defined(LINKSTATE)

Node::Node(const Node &rhs)
{
  number = rhs.number;
  context = rhs.context;
  bw = rhs.bw;
  lat = rhs.lat;
  linkTable = rhs.linkTable;

}



void Node::LinkHasBeenUpdated(const Link *l)
{
  cerr << *this<<": Link Update: "<<*l<<endl;
  const RoutingMessage * rmSend = new RoutingMessage(linkTable->UpdateLink(l), l->GetSrc(), l->GetDest(), l->GetLatency());
  SendToNeighbors(rmSend);
}


void Node::ProcessIncomingRoutingMessage(const RoutingMessage *m)
{
  cerr << *this << " Routing Message: "<<*m;
  if(linkTable->UpdateTable(m))
  {
    SendToNeighbors(m);
  }
}

void Node::TimeOut()
{
  cerr << *this << " got a timeout: ignored"<<endl;
}


Node *Node::GetNextHop(const Node *destination) {
  // if(number == destination->number) {
  //   return this;
  // }

  int nodes = (linkTable->table).size();

  vector<int>  dx(nodes, MAX);
  vector<int>  px(nodes, MAX);
  vector<bool> visited(nodes, false);

  int currentDistance = dx[number] = 0;
  visited[number] = true;
  px[number] = MAX;

  for(map<int, TableItem>::const_iterator i = linkTable->table[number].begin(); i != linkTable->table[number].end(); i++)
  {
    dx[i->first] = i->second.cost;
    px[i->first] = number;
  }
  
  for(int i = 0; i < nodes - 1; i++)
  {
    int minNode, minDistance = MAX;
    
    for(int i = 0; i < dx.size(); i++) 
    {
      if (visited[i] == false)
      {
        if(minDistance > dx[i]) 
        {
          minNode = i;
          minDistance = dx[i];
        }
      }
    }

    visited[minNode] = true;
    if(minDistance != MAX)
    {
      currentDistance = minDistance;
    }

    for(map<int, TableItem>::const_iterator i = linkTable->table[minNode].begin(); i != linkTable->table[minNode].end(); ++i)
    {
      if (dx[i->first] > (i->second.cost + currentDistance) && visited[i->first] == false)
      {
        dx[i->first] = i->second.cost + currentDistance;
        px[i->first] = minNode;
      }
    }
  }

  int currentNode = destination->number;
  int currentParent = px[destination->number];

  while (currentParent!=number)
  {
    currentNode = currentParent;
    currentParent = px[currentNode];
  }
  
  // build routing table
  int djikstraNode = destination->number;
  int djikstraParent = px[djikstraNode];
  while(djikstraParent != number)
  {
    djikstraNode = djikstraParent;
    djikstraParent = px[djikstraNode];
  }
  
  deque<Node*> *neighbors = this->GetNeighbors();
  for (deque<Node*>::const_iterator i = neighbors->begin(); i != neighbors->end(); ++i) 
  {
      if ((Node(currentNode, 0, 0, 0).Matches(**i)))
      {
          return new Node(**i);
      }
  }  
}


Table *Node::GetRoutingTable() const
{
  // WRITE
  return 0;
}


ostream & Node::Print(ostream &os) const
{
  os << "Node(number="<<number<<", lat="<<lat<<", bw="<<bw<<")";
  return os;
}
#endif


#if defined(DISTANCEVECTOR)

Node::Node(const Node &rhs)
{
  number = rhs.number;
  context = rhs.context;
  bw = rhs.bw;
  lat = rhs.lat;
  latencyTable = rhs.latencyTable;
}

void Node::UpdateNeighbors(unsigned dest, unsigned latency) 
{
  deque<Link*> *links = this->GetOutgoingLinks();
  deque<Node*> *nodes = this->GetNeighbors();
  RoutingMessage *message = new RoutingMessage(this->GetNumber(), dest, latency);

  Node *curr;
  for (deque<Link*>::iterator i = links->begin(); i != links->end(); ++i) {
    for (deque<Node*>::iterator j = nodes->begin(); j != nodes->end(); ++j) {
      if (Node((*i)->GetDest(), 0, 0, 0).Matches(**j)) {
        curr = *j;
        break;
      }
    }
    Event *event = new Event(context->GetTime() + (*i)->GetLatency(), ROUTING_MESSAGE_ARRIVAL, curr, message);
    context->PostEvent(event);
  }
  delete links;
  delete nodes;
}

void Node::LinkHasBeenUpdated(const Link *l)
{
  cerr << *this<<": Link Update: "<<*l<<endl;
  latencyTable->WriteTable(l->GetDest(), l->GetDest(), l->GetLatency()); //write new link to table
  UpdateNeighbors(l->GetDest(), l->GetLatency()); //update neighbors with the link
}


void Node::ProcessIncomingRoutingMessage(const RoutingMessage *m)
{
    Link *messageLink = context->FindMatchingLink(new Link(number, m->GetSrc(), 0, 0, 0));
    if (latencyTable->CheckLatency(m->GetDest(), m->GetSrc(), m->GetLatency() + messageLink->GetLatency())) 
    {
      latencyTable->WriteTable(m->GetDest(), m->GetSrc(), m->GetLatency() + messageLink->GetLatency());
      latencyTable->UpdateNeighbors(m->GetDest(), m->GetLatency() + messageLink->GetLatency());
    }
}

void Node::TimeOut()
{
  cerr << *this << " got a timeout: ignored"<<endl;
}


Node *Node::GetNextHop(const Node *destination) const
{
  for (deque<Node*>::const_iterator i = GetNeighbors()->begin(); i != GetNeighbors()->end(); ++i) 
  {
      if ((Node(latencyTable->GetNext(destination->number, 0, 0, 0).Matches(**i))) {
          return new Node(**i);
      }
  }
  return 0;
}

Table *Node::GetRoutingTable() const
{
  return latencyTable;
}


ostream & Node::Print(ostream &os) const
{
  os << "Node(number="<<number<<", lat="<<lat<<", bw="<<bw;
  return os;
}
#endif
