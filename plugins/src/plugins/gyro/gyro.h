/***************************************************************************
 *  gyro.h - Plugin for a gyro sensor on a model
 *
 *  Created: Tue Feb 04 14:42:29 2014
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
/** @class Gyro
   * Plugin for a gyro sensor on a model
   * @author Frederik Zwilling
   */
class Gyro : public ModelPlugin, public gazebo_rcll::ConfigurableAspect
{
public:
	///Constructor
	Gyro();

	///Destructor
	~Gyro();

	//Overridden ModelPlugin-Functions
	virtual void Load(physics::ModelPtr _parent, sdf::ElementPtr /*_sdf*/);
	virtual void OnUpdate(const common::UpdateInfo &);
	virtual void Reset();

private:
	/// Pointer to the model
	physics::ModelPtr model_;
	/// Pointer to the update event connection
	event::ConnectionPtr update_connection_;
	///Node for communication to fawkes
	transport::NodePtr node_;
	///name of the gyro and the communication channel
	std::string name_;

	///time variable to send in intervals
	double last_sent_time_;

	///time interval between to gyro msgs
	double send_interval_;

	//Gyro Stuff:
	///Sending Gyro-angle to fawkes:
	void send_gyro();

	///Publisher for GyroAngle
	transport::PublisherPtr gyro_pub_;
};
} // namespace gazebo
