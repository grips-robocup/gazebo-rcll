/***************************************************************************
 *  gps.h - provides ground truth position
 *
 *  Created: Tue Feb 04 15:05:07 2014
 *  Copyright  2014  Frederik Zwilling
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include <configurable/configurable.h>

#include <boost/bind.hpp>
#include <gazebo/common/common.hh>
#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/transport/transport.hh>
#include <list>
#include <stdio.h>
#include <string.h>

namespace gazebo {
/**
   * Provides ground Truth position
   * @author Frederik Zwilling
   */
class Gps : public ModelPlugin, public gazebo_rcll::ConfigurableAspect
{
public:
	Gps();
	~Gps();

	//Overridden ModelPlugin-Functions
	virtual void Load(physics::ModelPtr _parent, sdf::ElementPtr /*_sdf*/);
	virtual void OnUpdate(const common::UpdateInfo &);
	virtual void Reset();

private:
	/// Pointer to the gazbeo model
	physics::ModelPtr model_;
	/// Pointer to the update event connection
	event::ConnectionPtr update_connection_;
	///Node for communication to fawkes
	transport::NodePtr node_;
	///WorldNode for communication to fawkes
	transport::NodePtr world_node_;
	///name of the gps and the communication channel
	std::string name_;

	///config value used to ckeck if WorldNode should be published
	bool publish_world_node_;

	///time variable to send in intervals
	double last_sent_time_;

	//Gps Stuff:
	///Functions for sending information to fawkes:
	void send_position();

	///Publisher for GyroAngle
	transport::PublisherPtr gps_pub_;

	///Publisher for PuckPositions
	transport::PublisherPtr gps_world_pub_;
};
} // namespace gazebo
