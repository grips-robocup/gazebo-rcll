/***************************************************************************
 *  conveyor_vision.cpp - Plugin for a conveyor_vision sensor on a model
 *
 *  Created: Sun Jul 12 16:02:29 2015
 *  Copyright  2015  Randolph Maaßen
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

#include "conveyor_vision.h"

#include "../mps/mps.h"

#include <utils/misc/gazebo_api_wrappers.h>

#include <math.h>

using namespace gazebo;

// Register this plugin to make it available in the simulator
GZ_REGISTER_MODEL_PLUGIN(ConveyorVision)

ConveyorVision::ConveyorVision()
{
}

ConveyorVision::~ConveyorVision()
{
	printf("Destructing Conveyor Vision Plugin!\n");
}

/** on loading of the plugin
 * @param _parent Parent Model
 */
void
ConveyorVision::Load(physics::ModelPtr _parent, sdf::ElementPtr /*_sdf*/)
{
	// Store the pointer to the model
	this->model_ = _parent;

	//get the model-name
	this->name_ = model_->GetName();
	printf("Loading Conveyor Vision Plugin of model %s\n", name_.c_str());

	// Listen to the update event. This event is broadcast every
	// simulation iteration.
	this->update_connection_ =
	  event::Events::ConnectWorldUpdateBegin(boost::bind(&ConveyorVision::OnUpdate, this, _1));

	//Create the communication Node for communication with fawkes
	this->node_ = transport::NodePtr(new transport::Node());
	//the namespace is set to the model name!
	this->node_->Init(model_->GetWorld()->GZWRAP_NAME() + "/" + name_);

	//load config values
	number_pucks_                  = config->get_int("plugins/mps/number_pucks");
	belt_offset_side_              = config->get_float("plugins/mps/belt_offset_side");
	slide_offset_side_             = config->get_float("plugins/mps/slide_offset_side");
	detect_tolerance_              = config->get_float("plugins/mps/detect_tolerance");
	puck_size_                     = config->get_float("plugins/mps/puck_size");
	puck_height_                   = config->get_float("plugins/mps/puck_height");
	belt_length_                   = config->get_float("plugins/mps/belt_length");
	belt_height_                   = config->get_float("plugins/mps/belt_height");
	tag_height_                    = config->get_float("plugins/mps/tag_height");
	tag_size_                      = config->get_float("plugins/mps/tag_size");
	std::string topic_set_conveyor = config->get_string("plugins/gripper/topic-set-conveyor");

	//create publisher
	this->conveyor_pub_ =
	  this->node_->Advertise<llsf_msgs::ConveyorVisionResult>("~/RobotinoSim/ConveyorVisionResult/");
	offset_z_ = 0;

	//create subscriber
	set_conveyor_sub_ =
	  node_->Subscribe(topic_set_conveyor, &ConveyorVision::on_set_conveyor_msg, this);

	//init last sent time
	last_sent_time_      = model_->GetWorld()->GZWRAP_SIM_TIME().Double();
	this->send_interval_ = 0.05;
}

/** Called by the world update start event
 */
void
ConveyorVision::OnUpdate(const common::UpdateInfo & /*_info*/)
{
	//Send gyro information to Fawkes
	double time = model_->GetWorld()->GZWRAP_SIM_TIME().Double();
	if (time - last_sent_time_ > send_interval_) {
		last_sent_time_ = time;
		send_conveyor_result();
	}
}

/** on Gazebo reset
 */
void
ConveyorVision::Reset()
{
}

void
ConveyorVision::on_set_conveyor_msg(ConstIntPtr &msg)
{
	offset_z_ += ((float)msg->data()) / 1000.;
}

inline bool
is_machine(gazebo::physics::ModelPtr model)
{
	std::string name = model->GetName();
	return ((name.find("BS") != std::string::npos) || (name.find("SS") != std::string::npos)
	        || (name.find("CS") != std::string::npos) || (name.find("DS") != std::string::npos)
	        || (name.find("RS") != std::string::npos));
}

void
ConveyorVision::send_conveyor_result()
{
	if (!conveyor_pub_->HasConnections()) {
		return;
	}
	physics::LinkPtr camera_link = model_->GetLink("carologistics-robotino-3::conveyor_cam::link");
	if (!camera_link) {
		printf("Can't find conveyor camera\n");
		return;
	}

	physics::LinkPtr base_link = model_->GetLink("carologistics-robotino-3::base_link");
	if (!base_link) {
		printf("Can't find base_link on robotino model\n");
		return;
	}
	gzwrap::Pose3d camera_pose = camera_link->GZWRAP_WORLD_POSE();
	double look_pos_x = camera_pose.GZWRAP_POS_X + cos(camera_pose.GZWRAP_ROT_YAW) * SEARCH_AREA_REL_X
	                    - sin(camera_pose.GZWRAP_ROT_YAW) * SEARCH_AREA_REL_Y;
	double look_pos_y = camera_pose.GZWRAP_POS_Y + sin(camera_pose.GZWRAP_ROT_YAW) * SEARCH_AREA_REL_X
	                    + cos(camera_pose.GZWRAP_ROT_YAW) * SEARCH_AREA_REL_Y;
	gzwrap::Pose3d look_pose(look_pos_x, look_pos_y, BELT_HEIGHT, 0, 0, 0);

#if GAZEBO_MAJOR_VERSION >= 8
	gazebo::physics::Model_V models = model_->GetWorld()->Models();
#else
	gazebo::physics::Model_V models = model_->GetWorld()->GetModels();
#endif
	for (gazebo::physics::Model_V::iterator it = models.begin(); it != models.end(); it++) {
		gazebo::physics::ModelPtr model = *it;
		if (is_machine(model)
		    && look_pose.GZWRAP_POS.Distance(model->GZWRAP_WORLD_POSE().GZWRAP_POS
		                                     + gzwrap::Vector3d(0, 0, BELT_HEIGHT))
		         < RADIUS_DETECTION_AREA) {
			//check which side of the conveyor the bot is looking on
			gzwrap::Pose3d            mps_pose = model->GZWRAP_WORLD_POSE();
			const gzwrap::Quaterniond yaw_correction(0, 0, IGN_PI_2);
			//Calculate conveyor input position (positive X points twards conveyor mid-point)
			//        z up
			//       /
			//      x---> I=====O
			//      |
			//      y
			double conv_input_x = mps_pose.GZWRAP_POS_X + BELT_OFFSET_SIDE * cos(mps_pose.GZWRAP_ROT_YAW)
			                      - (BELT_LENGTH / 2 - PUCK_SIZE) * sin(mps_pose.GZWRAP_ROT_YAW);
			double conv_input_y = mps_pose.GZWRAP_POS_Y + BELT_OFFSET_SIDE * sin(mps_pose.GZWRAP_ROT_YAW)
			                      + (BELT_LENGTH / 2 - PUCK_SIZE) * cos(mps_pose.GZWRAP_ROT_YAW);
			const gzwrap::Vector3d conv_input_pose(conv_input_x, conv_input_y, BELT_HEIGHT);

			const gzwrap::Quaterniond conv_input_angle = mps_pose.GZWRAP_ROT_SUB(yaw_correction);

			gzwrap::Pose3d input_pose(conv_input_pose, conv_input_angle);

			//Calculate output conveyor position (positive X points twards conveyor mid-point)
			//                    z up
			//                   /
			//    I=====O  <--- x
			//                  |
			//                  y
			double conv_output_x = mps_pose.GZWRAP_POS_X + BELT_OFFSET_SIDE * cos(mps_pose.GZWRAP_ROT_YAW)
			                       + (BELT_LENGTH / 2 - PUCK_SIZE) * sin(mps_pose.GZWRAP_ROT_YAW);
			double conv_output_y = mps_pose.GZWRAP_POS_Y + BELT_OFFSET_SIDE * sin(mps_pose.GZWRAP_ROT_YAW)
			                       - (BELT_LENGTH / 2 - PUCK_SIZE) * cos(mps_pose.GZWRAP_ROT_YAW);
			const gzwrap::Vector3d conv_output_pose(conv_output_x, conv_output_y, BELT_HEIGHT);

			const gzwrap::Quaterniond conv_output_angle = mps_pose.GZWRAP_ROT_ADD(yaw_correction);

			gzwrap::Pose3d output_pose(conv_output_pose, conv_output_angle);

			//Calculate slide pose in case of an RS
			bool           is_RS = model->GetName().find("RS") != std::string::npos;
			gzwrap::Pose3d slide_pose(0, 0, 0, 0, 0, 0);
			if (is_RS) {
				double slide_x = mps_pose.GZWRAP_POS_X
				                 + (BELT_OFFSET_SIDE + SLIDE_OFFSET) * cos(mps_pose.GZWRAP_ROT_YAW)
				                 - (BELT_LENGTH / 2 - PUCK_SIZE) * sin(mps_pose.GZWRAP_ROT_YAW);
				double slide_y = mps_pose.GZWRAP_POS_Y
				                 + (BELT_OFFSET_SIDE + SLIDE_OFFSET) * sin(mps_pose.GZWRAP_ROT_YAW)
				                 + (BELT_LENGTH / 2 - PUCK_SIZE) * cos(mps_pose.GZWRAP_ROT_YAW);
				const gzwrap::Vector3d    slide_input_pose(slide_x, slide_y, BELT_HEIGHT);
				const gzwrap::Quaterniond slide_angle = mps_pose.GZWRAP_ROT_SUB(yaw_correction);
				slide_pose.Set(slide_input_pose, slide_angle);
			}

			gzwrap::Pose3d res_conv;
			gzwrap::Pose3d res_slide;
			gzwrap::Pose3d base_link_pose = base_link->GZWRAP_WORLD_POSE();
			if (input_pose.GZWRAP_POS.Distance(camera_pose.GZWRAP_POS)
			    < output_pose.GZWRAP_POS.Distance(camera_pose.GZWRAP_POS)) {
				//printf("looking at input\n");
				res_conv = input_pose - base_link_pose;
				if (is_RS) {
					res_slide = slide_pose - base_link_pose;
				}
			} else {
				//printf("looking at output\n");
				res_conv = output_pose - base_link_pose;
			}
			//get position in the camera frame
			llsf_msgs::ConveyorVisionResult conv_msg;
			llsf_msgs::Pose3D *             pose = new llsf_msgs::Pose3D();
			pose->set_x(res_conv.GZWRAP_POS_X);
			pose->set_y(res_conv.GZWRAP_POS_Y);
			pose->set_z(res_conv.GZWRAP_POS_Z);
			pose->set_ori_x(res_conv.GZWRAP_ROT_X);
			pose->set_ori_y(res_conv.GZWRAP_ROT_Y);
			pose->set_ori_z(res_conv.GZWRAP_ROT_Z);
			pose->set_ori_w(res_conv.GZWRAP_ROT_W);
			conv_msg.set_allocated_conveyor(pose);
			if (is_RS) {
				pose = new llsf_msgs::Pose3D();
				pose->set_x(res_slide.GZWRAP_POS_X);
				pose->set_y(res_slide.GZWRAP_POS_Y);
				pose->set_z(res_slide.GZWRAP_POS_Z);
				pose->set_ori_x(res_slide.GZWRAP_ROT_X);
				pose->set_ori_y(res_slide.GZWRAP_ROT_Y);
				pose->set_ori_z(res_slide.GZWRAP_ROT_Z);
				pose->set_ori_w(res_slide.GZWRAP_ROT_W);
				conv_msg.set_allocated_slide(pose);
			}
			//send
			conveyor_pub_->Publish(conv_msg);
			break;
		}
	}
}
