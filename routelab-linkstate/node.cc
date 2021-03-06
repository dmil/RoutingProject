#include "node.h"
#include "context.h"
#include "error.h"
#define MAX 9999999

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

Node::Node(const Node &rhs)
{
  #if defined(DISTANCEVECTOR)
  number = rhs.number;
  context = rhs.context;
  bw = rhs.bw;
  lat = rhs.lat;
  latencyTable = rhs.latencyTable;
  #endif

  #if defined(LINKSTATE)
  number = rhs.number;
  context = rhs.context;
  bw = rhs.bw;
  lat = rhs.lat;
  linkTable = rhs.linkTable;
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

deque<Link*> *Node::GetOutgoingLinks() { return context->GetOutgoingLinks(this); }



Node::~Node()
{}

// Implement these functions  to post an event to the event queue in the event simulator
// so that the corresponding node can recieve the ROUTING_MESSAGE_ARRIVAL event at the proper time
void Node::SendToNeighbors(const RoutingMessage *m)
{
  //iterate through neighbors and send each the routing message
  deque <Node*> *neighbors = GetNeighbors();
  for(deque<Node*>:: iterator itr = neighbors->begin(); itr != neighbors->end(); ++itr)
  {
    SendToNeighbor(*itr,m);
  }
}

void Node::SendToNeighbor(const Node *n, const RoutingMessage *m)
{
  //Extract link from routing message and put in const - find that actual link and put it in matchedLink
  const Link * rmLink = new Link(number, n->number, NULL, 0, 0);
  Link * matchedLink = context->FindMatchingLink(rmLink);

  //Post the event for other nodes to receive
  if(matchedLink != 0) {
   context->PostEvent(new Event(context->GetTime()+matchedLink->GetLatency(),ROUTING_MESSAGE_ARRIVAL, new Node(*n), new RoutingMessage(*m)));
  }

  delete rmLink;
}

deque<Node*> *Node::GetNeighbors()
{ 
  return context->GetNeighbors(this); 
}

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

void Node::LinkHasBeenUpdated(const Link *l)
{
  cerr << *this<<": Link Update: "<<*l<<endl;
  //update link in the table
  const RoutingMessage * rmSend = new RoutingMessage(linkTable->UpdateLink(l), l->GetSrc(), l->GetDest(), l->GetLatency());
  //send routing message to neighbors
  SendToNeighbors(rmSend);
}


void Node::ProcessIncomingRoutingMessage(const RoutingMessage *m)
{
  cerr << *this << " Routing Message: "<<*m;
  //if there is someting new to be written to table (shorter path found) -> send the update to the neighors
  if(linkTable->UpdateTable(m))
    SendToNeighbors(m);
}

void Node::TimeOut()
{ 
  cerr << *this << " got a timeout: ignored"<<endl;
}


Node *Node::GetNextHop(const Node *destination) {

  int nodes = (linkTable->table).size();// Total # of Nodes

  /*Build 3 tables to store min values for Djikstra's Algorithm*/
  vector<int>  dx(nodes, MAX); //D(x) = Distances 
  vector<int>  px(nodes, MAX); //P(x) = Parents 
  vector<bool> visited(nodes, false);

  /*"Follow" current node*/
  int currentDistance = dx[number] = 0;
  visited[number] = true;
  px[number] = MAX;
  //Add Current Node's Neighbors to the table
  for(map<int, TableItem>::const_iterator i = linkTable->table[number].begin(); i != linkTable->table[number].end(); i++)
  {
    dx[i->first] = i->second.cost;
    px[i->first] = number;
  }
  

  for(int i = 0; i < nodes - 1; i++)//for all but the initial node
  {
    /*Find the node with the minnimum distance*/
    int nodeToFollow, minDistance = MAX;
    
    for(int i = 0; i < dx.size(); i++) 
    {
      if (visited[i] == false)
      {
        if(minDistance > dx[i]) 
        {
          nodeToFollow = i;
          minDistance = dx[i];
        }
      }
    }

    /*Follow Closest Node*/
    visited[nodeToFollow] = true;
    if(minDistance != MAX)
    {
      currentDistance = minDistance;
    }

    /*Add Closest Node's Neighbors to Table*/
    for(map<int, TableItem>::const_iterator i = linkTable->table[nodeToFollow].begin(); i != linkTable->table[nodeToFollow].end(); ++i)
    {
      if (dx[i->first] > (i->second.cost + currentDistance) && visited[i->first] == false)
      {
        dx[i->first] = i->second.cost + currentDistance;
        px[i->first] = nodeToFollow;
      }
    }
  }

  /*Backtrace to find next hop*/
  int currentNode = destination->number;
  int currentParent = px[destination->number];

  while (currentParent!=number)
  {
    currentNode = currentParent;
    currentParent = px[currentNode];
  }
  
  /* Find Next Hop # */
  int djikstraNode = destination->number;
  int djikstraParent = px[djikstraNode];
  while(djikstraParent != number)
  {
    djikstraNode = djikstraParent;
    djikstraParent = px[djikstraNode];
  }
  
  /*Find actual node to Return*/
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
  return linkTable;
}


ostream & Node::Print(ostream &os) const
{
  os << "Node(number="<<number<<", lat="<<lat<<", bw="<<bw<<")";
  return os;
}
#endif


#if defined(DISTANCEVECTOR)


void Node::LinkHasBeenUpdated(const Link *l)
{
  cerr << *this<<": Link Update: "<<*l<<endl;
  //write new link to the table
  latencyTable->WriteTable(l->GetDest(), l->GetDest(), l->GetLatency());

  deque<Link*> *links = GetOutgoingLinks();
  deque<Node*> *nodes = GetNeighbors();
  //Iterate through links and neighbors to find matches
  for (deque<Link*>::iterator i = links->begin(); i != links->end(); ++i) {
    for (deque<Node*>::iterator ii = nodes->begin(); ii != nodes->end(); ++ii) {
      if (Node((*i)->GetDest(), 0, 0, 0).Matches(**ii)) {//a neighbor matches an outgoing link
        //post a routing message event for the link for other nodes to read
        context->PostEvent(new Event(context->GetTime() + (*i)->GetLatency(), ROUTING_MESSAGE_ARRIVAL, *ii, new RoutingMessage(number, l->GetDest(), l->GetLatency())));
        break;
      }
    }
  }

  delete links;
  delete nodes;
}


void Node::ProcessIncomingRoutingMessage(const RoutingMessage *m)
{
    Link *messageLink = context->FindMatchingLink(new Link(number, m->GetSource(), 0, 0, 0));
    if (latencyTable->CheckLatency(m->GetDestination(), m->GetSource(), m->GetLatency() + messageLink->GetLatency())) 
    {
      latencyTable->WriteTable(m->GetDestination(), m->GetSource(), m->GetLatency() + messageLink->GetLatency());

        deque<Link*> *links = GetOutgoingLinks();
        deque<Node*> *nodes = GetNeighbors();
        //Iterate through links and neighbors to find matches
        for (deque<Link*>::iterator i = links->begin(); i != links->end(); ++i) {
          for (deque<Node*>::iterator ii = nodes->begin(); ii != nodes->end(); ++ii) {
            if (Node((*i)->GetDest(), 0, 0, 0).Matches(**ii)) {//a neighbor matches an outgoing link
              //post a routing message event for the link for other nodes to read
              context->PostEvent(new Event(context->GetTime() + (*i)->GetLatency(), ROUTING_MESSAGE_ARRIVAL, *ii, new RoutingMessage(number, m->GetDestination(), m->GetLatency() + messageLink->GetLatency())));
              break;
            }
          }
        }

        delete links;
        delete nodes;

    }
}

void Node::TimeOut()
{
  cerr << *this << " got a timeout: ignored"<<endl;
}

Node *Node::GetNextHop(const Node *destination)
{

  deque<Node*> *nodes = GetNeighbors();
  unsigned ret = latencyTable->GetNext(destination->GetNumber());
  for (deque<Node*>::const_iterator i = nodes->begin(); i != nodes->end(); ++i) {
      if ((Node(ret, 0, 0, 0).Matches(**i))) {//if the node exists, return node
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
