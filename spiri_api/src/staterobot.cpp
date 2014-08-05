#include <spiri_api/staterobot.h>
#include <iostream>
#include <ctime>
#include <boost/timer.hpp>
#include <sstream>
Staterobot::Staterobot()
{
    int dummy_argc=0;
    ros::init((int&)dummy_argc,NULL,"spiri_api");
    ros::NodeHandle n;
    success_plan=false;
    success_execution=false;


}

geometry_msgs::Quaternion Staterobot::get_imu()
{

    imu=ros::topic::waitForMessage<sensor_msgs::Imu>("/raw_imu");
    return imu->orientation;
}

boost::python::list Staterobot::get_imu_python()
{
    geometry_msgs::Quaternion imu_data=this->get_imu();
    std::vector<double> v(3);
    v[0]=imu_data.x;
    v[1]=imu_data.y;
    v[2]=imu_data.z;
    return moveit::py_bindings_tools::listFromDouble(v);
}

geometry_msgs::Pose Staterobot::get_state()
{
    odom=ros::topic::waitForMessage<nav_msgs::Odometry>("/ground_truth/state");
    return odom->pose.pose;

}

boost::python::list Staterobot::get_state_python()
{
    geometry_msgs::Pose pose_data=this->get_state();
    std::vector<double> v(7);
    v[0]=pose_data.position.x;
    v[1]=pose_data.position.y;
    v[2]=pose_data.position.z;
    v[3]=pose_data.orientation.x;
    v[4]=pose_data.orientation.y;
    v[5]=pose_data.orientation.z;
    v[6]=pose_data.orientation.w;
    return moveit::py_bindings_tools::listFromDouble(v);
}


float Staterobot::get_height_pressure()
{
    pressure=ros::topic::waitForMessage<geometry_msgs::PointStamped>("pressure_height");
    return pressure->point.z;
}

sensor_msgs::NavSatFixConstPtr Staterobot::get_gps_data()
{
    gps=ros::topic::waitForMessage<sensor_msgs::NavSatFix>("/fix");
    // @todo Need to return three floats or some sort of structure
    return gps;
}

boost::python::list Staterobot::get_gps_data_python()
{
    sensor_msgs::NavSatFixConstPtr gps_data=this->get_gps_data();
    std::vector<double> v(3);
    v[0]=gps_data->latitude;
    v[1]=gps_data->longitude;
    v[2]=gps_data->altitude;
    return moveit::py_bindings_tools::listFromDouble(v);
}

geometry_msgs::Vector3 Staterobot::get_gps_vel()
{
    gps_vel=ros::topic::waitForMessage<geometry_msgs::Vector3Stamped>("/fix_velocity");
    return gps_vel->vector;
}

boost::python::list Staterobot::get_gps_vel_python()
{
    geometry_msgs::Vector3 vel_data=this->get_gps_vel();
    std::vector<double> v(3);
    v[0]=vel_data.x;
    v[1]=vel_data.y;
    v[2]=vel_data.z;
    return moveit::py_bindings_tools::listFromDouble(v);

}


float Staterobot::get_height_altimeter()
{
    altimeter=ros::topic::waitForMessage<hector_uav_msgs::Altimeter>("/altimeter");
    return altimeter->altitude;
}

cv::Mat Staterobot::get_left_image()

{
    ros::NodeHandle n;
    n.setCallbackQueue(&image_queue);
    image_transport::ImageTransport it(n);
    sub=it.subscribe("/stereo/left/image_raw", 1, &Staterobot::image_left_callback,this);



    image_queue.callAvailable(ros::WallDuration(1.0));
    return this->left_image;

}


std::string Staterobot::get_left_image_python()

{
    cv::Mat mat=this->get_left_image();
    cv::Size size = mat.size();
    int total = size.width * size.height * mat.channels();

    std::vector<uchar> data(mat.ptr(),mat.ptr()+total);
    std::string image(data.begin(),data.end());

    return image;

}



void Staterobot::image_left_callback(const sensor_msgs::ImageConstPtr & msg)
{


    left_image=cv_bridge::toCvShare(msg,"bgr8")->image;

}
cv::Mat Staterobot::get_right_image()

{
    // @todo there is a better way to do this. Callback to a subscriber. No need to create a node

    // @todo the image callback needs to be called once or else it return an older image.
    ros::NodeHandle n;
    n.setCallbackQueue(&image_queue);
    image_transport::ImageTransport it(n);
    sub=it.subscribe("/stereo/right/image_raw", 1, &Staterobot::image_right_callback,this);



    image_queue.callAvailable(ros::WallDuration(1.0));
    return this->right_image;

}

std::string Staterobot::get_right_image_python()

{
    cv::Mat mat=this->get_right_image();
    cv::Size size = mat.size();
    int total = size.width * size.height * mat.channels();

    std::vector<uchar> data(mat.ptr(),mat.ptr()+total);
    std::string image(data.begin(),data.end());

    return image;

}
void Staterobot::image_right_callback(const sensor_msgs::ImageConstPtr & msg)
{


    right_image=cv_bridge::toCvShare(msg,"bgr8")->image;

}
cv::Mat Staterobot::get_bottom_image()

{
    ros::NodeHandle n;
    n.setCallbackQueue(&image_queue);
    image_transport::ImageTransport it(n);
    sub=it.subscribe("/downward_cam/camera/image", 1, &Staterobot::image_bottom_callback,this);



    image_queue.callAvailable(ros::WallDuration(1.0));
    return this->bottom_image;

}

std::string Staterobot::get_bottom_image_python()

{
    cv::Mat mat=this->get_bottom_image();
    cv::Size size = mat.size();
    int total = size.width * size.height * mat.channels();

    std::vector<uchar> data(mat.ptr(),mat.ptr()+total);
    std::string image(data.begin(),data.end());

    return image;

}
void Staterobot::image_bottom_callback(const sensor_msgs::ImageConstPtr & msg)
{


    bottom_image=cv_bridge::toCvShare(msg,"bgr8")->image;

}

void Staterobot::save_image(const std::string path="",const std::string camera="")
{
    //std::cout<<path;
    cv::Mat image;
    if(camera=="left")
    {

            image=get_left_image();
    }
    else if(camera=="right")
    {
        image=get_right_image();
    }
    else if(camera=="bottom")
    {
        image=get_bottom_image();
    }
    else
    {
        std::cout<<"Please pass a valid camera type";

    }


    //cv::Mat image=get_left_image();

    cv::imwrite(path,image);
}
bool Staterobot::wait_goal()
{
    bool param;
    ros::param::get("execution",param);
    return param;


}

void Staterobot::callback_goal(const action_controller::MultiDofFollowJointTrajectoryActionResultPtr& msg)
{
    ROS_INFO("here");
    //std::cout<<"here";
    status=true;

}

bool Staterobot::send_goal(float x,float y,float z, bool relative=false)
{
    ros::AsyncSpinner spinner(1);
    spinner.start();
    moveit::planning_interface::MoveGroup group("spiri");
    group.setPlannerId("PRMkConfigDefault");
    current_state=get_state();
    transform.translation.x=current_state.position.x;
    transform.translation.y=current_state.position.y;
    transform.translation.z=current_state.position.z;
    transform.rotation.x=current_state.orientation.x;
    transform.rotation.y=current_state.orientation.y;
    transform.rotation.z=current_state.orientation.z;
    transform.rotation.w=current_state.orientation.w;
    start_state.multi_dof_joint_state.joint_names.push_back("virtual_join");
    start_state.multi_dof_joint_state.transforms.push_back(transform);
    start_state.joint_state.header.frame_id="/nav";
    start_state.multi_dof_joint_state.header.frame_id="/nav";
    group.setStartState(start_state);
    // after setting the start state send the goal
    group.getCurrentState()->copyJointGroupPositions(group.getCurrentState()->getRobotModel()->getJointModelGroup(group.getName()), group_variable_values);
    if (relative==true)
    {

        group_variable_values[0]=current_state.position.x+x;
        group_variable_values[1]=current_state.position.y+y;
        group_variable_values[2]=current_state.position.z+z;
    }
    else
    {
        group_variable_values[0]=x;
        group_variable_values[1]=y;
        group_variable_values[2]=z;
    }
    group.setJointValueTarget(group_variable_values);

    moveit::planning_interface::MoveGroup::Plan my_plan;
    // @todo Can combine plan and execute into one by using move.
    success_plan = group.plan(my_plan);
    if(success_plan==1)
    {
            ROS_INFO("Going to execute");
            success_execution=group.asyncExecute(my_plan);
    }
    else
    {
        ROS_INFO("Couldn't find a valid plan");
    }
    ROS_INFO("out of this loop");
    spinner.stop();
    return success_execution;

}

bool Staterobot::send_goal_python(boost::python::list &values)
{
    std::vector<double> v(3);
    v=moveit::py_bindings_tools::doubleFromList(values);
    bool flag=this->send_goal(v[0],v[1],v[2],false);
    return flag;
}

bool Staterobot::send_goal_python_relative(boost::python::list &values)
{
    std::vector<double> v(3);
    v=moveit::py_bindings_tools::doubleFromList(values);
    bool flag=this->send_goal(v[0],v[1],v[2],true);
    return flag;
}

void Staterobot::send_vel(float x,float y,float z)
{

    ROS_INFO("testing");
    ros::NodeHandle nh;
    bool latch=1;
    ros::Publisher vel_chatter = nh.advertise<geometry_msgs::Twist>("cmd_vel",1,latch);


    geometry_msgs::Twist vel;

    vel.linear.x=x;
    vel.linear.y=y;
    vel.linear.z=z;

    vel_chatter.publish(vel);
    // this is hack but it looks like it is not possible in ROS
    sleep(1.0);
    //ros::spinOnce();

}

void Staterobot::send_vel_python(boost::python::list &val)
{
    std::vector<double> v(3);
    v=moveit::py_bindings_tools::doubleFromList(val);
    this->send_vel(v[0],v[1],v[2]);
}


