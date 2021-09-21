/***************************************************************************
 *  tag-vision.h - provides ground truth tag-vision
 *
 *  Created: Mon Mar 30 16:15:38 2015
 *  Copyright  2015  Frederik Zwilling
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
#include <llsf_msgs/MachineInfo.pb.h>
#include <utils/misc/gazebo_api_wrappers.h>

#include <boost/bind.hpp>
#include <gazebo/common/common.hh>
#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/transport/transport.hh>
#include <list>
#include <map>
#include <stdio.h>
#include <string>

//config values
#define TOPIC_TAG_SUFFIX config->get_string("plugins/tag-vision/topic_tag_suffix").c_str()
#define TAG_VISION_RESULT_TOPIC \
	config->get_string("plugins/tag-vision/tag_vision_result_topic").c_str()
#define SEND_INTERVAL config->get_float("plugins/tag-vision/send_interval")
#define SEARCH_FOR_TAGS_INTERVAL config->get_int("plugins/tag-vision/search_for_tags_interval")
#define MAX_VIEW_DISTANCE config->get_int("plugins/tag-vision/max_view_distance")
#define CAMERA_FOV config->get_float("plugins/tag-vision/camera_fov")

namespace gazebo {
/**
   * Provides ground Truth position
   * @author Frederik Zwilling
   */
class TagVision : public ModelPlugin, public gazebo_rcll::ConfigurableAspect
{
public:
	TagVision();
	~TagVision();

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
	///Node for communication in gazebo
	transport::NodePtr world_node_;
	///name of the communication channel and the sensor
	std::string name_;

	///time variable to send in intervals
	double last_sent_time_;
	double last_searched_for_new_tags_time_;

	//robot position
	gzwrap::Pose3d link_pose_;

	/// Pointer to the link where the camera should be
	physics::LinkPtr link_;

	///Subscriber to get tag-positions
	std::map<physics::ModelPtr, gzwrap::Pose3d> tag_poses_;
	///Publisher for Detected tags
	transport::PublisherPtr result_pub_;

	int get_tag_id_from_name(std::string name);
};
} // namespace gazebo
