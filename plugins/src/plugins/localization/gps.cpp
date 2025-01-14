/***************************************************************************
 *  gps.cpp - Provides ground Truth position
 *
 *  Created: Tue Feb 04 15:06:06 2014
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

#include "gps.h"

#include <utils/misc/gazebo_api_wrappers.h>

#include <math.h>

using namespace gazebo;

// Register this plugin to make it available in the simulator
GZ_REGISTER_MODEL_PLUGIN(Gps)

///Constructor
Gps::Gps()
{
}
///Destructor
Gps::~Gps()
{
	printf("Destructing Gps Plugin!\n");
}

/** on loading of the plugin
 * @param _parent Parent Model
 */
void
Gps::Load(physics::ModelPtr _parent, sdf::ElementPtr /*_sdf*/)
{
	// Store the pointer to the model
	this->model_ = _parent;

	//get the model-name
	this->name_ = model_->GetName();
	printf("Loading Gps Plugin of model %s\n", name_.c_str());

	// Listen to the update event. This event is broadcast every
	// simulation iteration.
	this->update_connection_ =
	  event::Events::ConnectWorldUpdateBegin(boost::bind(&Gps::OnUpdate, this, _1));

	//Create the communication Node for communication with fawkes
	this->node_ = transport::NodePtr(new transport::Node());

	//the namespace is set to the model name!
	this->node_->Init(model_->GetWorld()->GZWRAP_NAME() + "/" + name_);

	//init last sent time
	last_sent_time_ = model_->GetWorld()->GZWRAP_SIM_TIME().Double();

	//create publisher
	this->gps_pub_ = this->node_->Advertise<msgs::Pose>("~/gazsim/gps/");

	//check if WorldNode should be published
	publish_world_node_ = config->get_bool("plugins/enable-public-object-pose-publisher");

	if (publish_world_node_) {
		//Create the communication WorldNode for communication with fawkes
		this->world_node_ = transport::NodePtr(new transport::Node());

		//the namespace is set to WorldSpace!
		this->world_node_->Init(model_->GetWorld()->GZWRAP_NAME());

		//create WorldNodePublisher
		this->gps_world_pub_ = this->world_node_->Advertise<msgs::Pose>("~/gazsim/gps/");
	}
}

/** Called by the world update start event
 */
void
Gps::OnUpdate(const common::UpdateInfo & /*_info*/)
{
	//Send position information to Fawkes
	double time = model_->GetWorld()->GZWRAP_SIM_TIME().Double();
	if (time - last_sent_time_ > (1.0 / 10.0)) {
		last_sent_time_ = time;
		send_position();
	}
}

/** on Gazebo reset
 */
void
Gps::Reset()
{
}

/** Sending position to Fawkes
 * 
 */
void
Gps::send_position()
{
	//build message
	msgs::Pose posMsg;
	posMsg.set_name(this->name_);
	posMsg.mutable_position()->set_x(this->model_->GZWRAP_WORLD_POSE().GZWRAP_POS_X);
	posMsg.mutable_position()->set_y(this->model_->GZWRAP_WORLD_POSE().GZWRAP_POS_Y);
	posMsg.mutable_position()->set_z(this->model_->GZWRAP_WORLD_POSE().GZWRAP_POS_Z);
	posMsg.mutable_orientation()->set_x(this->model_->GZWRAP_WORLD_POSE().GZWRAP_ROT_X);
	posMsg.mutable_orientation()->set_y(this->model_->GZWRAP_WORLD_POSE().GZWRAP_ROT_Y);
	posMsg.mutable_orientation()->set_z(this->model_->GZWRAP_WORLD_POSE().GZWRAP_ROT_Z);
	posMsg.mutable_orientation()->set_w(this->model_->GZWRAP_WORLD_POSE().GZWRAP_ROT_W);

	//send
	if (gps_pub_->HasConnections()) {
		gps_pub_->Publish(posMsg);
	}
	if (publish_world_node_ && gps_world_pub_->HasConnections()) {
		gps_world_pub_->Publish(posMsg);
	}
}
