#pragma once

#include <string>
#include <vector>
#include <map>

#include "../../common/network/ConnectionEndpoint.h"
#include "../../common/Player.h"
#include "IIdentifiable.h"
#include "StemCellServer.h"

using namespace std;

class PlayerServer :
	public Player,
	public ConnectionEndpoint,
	public IIdentifiable
{
public:
	PlayerServer(string name, boost::asio::ip::udp::endpoint endpoint, Vec3f position) :
		Player(IIdentifiable::getNewId(), name),  
		ConnectionEndpoint(endpoint),
		stemCell(new StemCellServer(position, 0.f))
	{ }
	
	const boost::asio::ip::udp::endpoint & getEndpoint() const { return m_endpoint; }
	StemCell const & getStemCell() const { return * stemCell; }

private:
	StemCellServer * stemCell;
};