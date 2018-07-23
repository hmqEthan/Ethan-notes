





	//initialize units of sd, and make sd.fops=msm_sensor_init_subdev_ops;
	v4l2_subdev_init(&s_init->msm_sd.sd, &msm_sensor_init_subdev_ops);
	//change name, dev_priv, internal_ops, flags, entity, entity.type, entity.group_id, entity.name, close_seq
	snprintf(s_init->msm_sd.sd.name, sizeof(s_init->msm_sd.sd.name), "%s",
		"msm_sensor_init");
	v4l2_set_subdevdata(&s_init->msm_sd.sd, s_init);//set sd.dev_priv = s_init
	s_init->msm_sd.sd.internal_ops = &msm_sensor_init_internal_ops;
	s_init->msm_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	media_entity_init(&s_init->msm_sd.sd.entity, 0, NULL, 0);//initialize sd.entity
	s_init->msm_sd.sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	s_init->msm_sd.sd.entity.group_id = MSM_CAMERA_SUBDEV_SENSOR_INIT;
	s_init->msm_sd.sd.entity.name = s_init->msm_sd.sd.name;
	s_init->msm_sd.close_seq = MSM_SD_CLOSE_2ND_CATEGORY | 0x6;
	//register sd	
	
	ret = msm_sd_register(&s_init->msm_sd);
	/*msm_sd_register will change value of sd.v4l2_dev, sd.entity.parent, sd.entity.entity_id;
	if V4L2_SUBDEV_FL_HAS_DEVNODE flag is set, a node should be build by register a video_device.
	1.v4l2_device_register_subdev(msm_v4l2_dev, sd), which will 
		make sd->v4l2_dev point to msm_v4l2_dev, 
		build connect between sd.entity and msm_v4l2_dev->mdev by sd.entity.parent and sd.entity.entity_id.
	2, build a video device, use __video_register_device() method to make a v4l2_subdev cdev node in /dev/, 
	make floder in /sys/device/platfor/
	*/

	
##############  __msm_sd_register_subdev()分析     ##########################
/*msm的subdev的注册，主要是向全局变量msm_v4l2_dev注册subdev；
	建立一个subdev类型的video4linux设备，使该设备driver_data指向subdev；
	使subdev得devnode指向该设备。
	向系统注册该设备。*/
static inline int __msm_sd_register_subdev(struct v4l2_subdev *sd);
{
	v4l2_device_register_subdev(msm_v4l2_dev, sd);
	
	if(!(sd->flags & V4L2_SUBDEV_FL_HAS_DEVNODE))
			return rc;

	struct video_device *vdev = malloc();
	video_set_drvdata(vdev, sd);//vdev->dev->driver_data
	strlcpy(vdev->name, sd->name, sizeof(vdev->name));//vdev->name
	vdev->v4l2_dev = msm_v4l2_dev;
	vdev->fops = msm_cam_get_v4l2_subdev_fops_ptr();
	vdev->release = msm_sd_unregister_subdev;
	rc = __video_register_device(vdev, VFL_TYPE_SUBDEV, -1, 1, sd->owner);

	sd->entity.name = video_device_node_name(vdev);
	sd->devnode = vdev;
}


/*向v4l2_device注册subdev，主要是建立v4l2_dev和sd之间的联系，两者的互指，sd的entity和v4l2_dev的mdev的关系*/
int v4l2_device_register_subdev(struct v4l2_device *v4l2_dev,
				struct v4l2_subdev *sd);
{
	sd->v4l2_dev = v4l2_dev;
#if defined(CONFIG_MEDIA_CONTROLLER)
	/*Register the entity*/
	if(v4l2_dev->mdev)
			media_device_register_entity(v4l2_dev->mdev, sd->entity);
#endif
	list_add_tail(&sd->list, &v4l2_subdevs);
}



/*只有media_device_register和__video_register_device会创建cdev和sysfs目录，
v4l2_device_register是建立和device的联系，
v4l2_device_register_subdev是建立和v4l2_device的联系。
这一点可以由这些函数的形参列表来记忆（并非推断），前两者形参中描述设备的只有一个，
因此是建立该设备与系统的联系，后两者形参中有2个设备，因此是建立这两个设备的联系。*/

/*register是注册，登记的意思，其实登记比较容易理解。*/
/*register sth with sth 将某向某登记。*/
/*v4l2-device.h中，v4l2_device_register_subdev的注释是register subdev with v4l2_device，意思是该函数将subdev
	向v4l2_device登记。*/
/*__vedio_device_register函数中，有注释 part 4 register the device with sysfs。*/




/*登记一个video4linux设备，video_device描述了一个video4linux设备。*/
/*video_device有多种类型，如frame grabber(帧获取器), radio card(射频卡), subdevice，用vfl_type表示。我们
用的是subdevice类型。*/
/*video_device与v4l2_subdev除了上面说的类型，还有什么样的关联呢？video->dev->driver_data = subdev*/
int __video_register_device(struct video_device *vdev, int type, int nr,
		int warn_if_nr_in_use, struct module *owner);
			
############## #end  __msm_sd_register_subdev()分析     ##########################



#################        驱动架构分析             ############

共建立2个/dev/media，2个/dev/video，2个/dev/v4l2-subdev。
msm_v4l2->mdev共有3个entity，其中1个在video_device中，2个在v4l2_subdev中。
如果注册的是subdev，则需用到__msm_sd_register_subdev函数。由字符设备对其调用路线是cdev.ops --> vdev.ops --> subdev.ops;在cdev的操作
函数中，通过file得到vdev，调用vdev.ops，在vdev.ops中，通过vdev得到subdev调用subdev.ops。






#### camera.c #####
int camera_init_v4l2(struct device *dev, unsigned int *session);
以下设置主要由media_device_register()，v4l2_device_register(), video_device_register()函数完成。
media_device{
	model = "msm-camera";
	dev->name = "/dev/media+num"
	cdev.ops = &media_devnode_fops;
};
media_device的entity的个数就是camera的个数，这一点由get_num_of_cameras可知。get_num_of_camera还会把所有的entity的name存入g_cam_ctrl.video_dev_name中。
video_device{
	name = "msm-sensor";
	fops = &camera_v4l2_fops;
	ioctl_ops = &camera_v4l2_ioctl_ops;
	num;                                 //session_id = num;
	cdev.ops = &v4l2_fops;
	dev->name = "/dev/video+num"
	vfl_type = VFL_TYPE_GRABBER;
	entity.name = "/dev/video+num";
	entity.type = MEDIA_ENT_T_DEVNODE_V4L;
	entity.group_id = QCAMERA_VNODE_GROUP_ID;
	entity.parent = media_device;
};
由mm_camera_open可知，cam_idx和session_id的值都是video_device.num。
my_obj->ctrl_fd = open("/dev/video+num", O_RDWR | O_NONBLOCK);
这个/dev/video是做什么的呢？
	mm_camera_get_session_id(my_obj, &my_obj->sessionid);获取session_id即为vdev->num
		--> ioctl(my_obj->ctrl_fd, VIDIOC_G_CTRL, &control);
	mm_camera_evt_sub(mm_camera_obj_t * my_obj, uint8_t reg_flag);
		sub.type = MSM_CAMERA_V4L2_EVENT_TYPE;
    	sub.id = MSM_CAMERA_MSM_NOTIFY;
		--> ioctl(my_obj->ctrl_fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
		只是添加到链表中就算是订阅了？

##### msm.c #####
static int msm_probe(struct platform_device *pdev);
以下设置主要由media_device_register()，v4l2_device_register(), video_device_register()函数完成。
msm_v4l2_dev->mdev{
	model = "msm-config";
	dev->name = "/dev/media+num"
	cdev.ops = &media_devnode_fops;
};
video_device{
	name = "msm-config";
	fops = &msm_fops;
	ioctl_ops = &g_msm_ioctl_ops;
	num;                                 
	cdev.ops = &v4l2_fops;
	dev->name = "/dev/video+num"
	vfl_type = VFL_TYPE_GRABBER;
	entity.name = "/dev/video+num";
	entity.type = MEDIA_ENT_T_DEVNODE_V4L;
	entity.group_id = QCAMERA_VNODE_GROUP_ID;
	entity.parent = msm_v4l2_dev->mdev;
};

##### msm_sensor_driver.c #####
static int32_t msm_sensor_driver_create_i2c_v4l_subdev(struct msm_sensor_ctrl_t *s_ctrl);
以下设置主要由__msm_sd_register_subdev()函数完成。
实体：
v4l2_subdev{
	name = s_ctrl->sensordata->sensor_name;
	v4l2_dev = msm_v4l2_dev;
	ops = hsm_subdev_ops;
	flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	entity.group_id = MSM_CAMERA_SUBDEV_SENSOR;
	entity.name = "/dev/v4l-subdev+num";
	entity.parent = msm_v4l2_dev->mdev;
	devnode = video_device;
};
节点：
video_device{
	name = s_ctrl->sensordata->sensor_name;
	fops = &v4l2_subdev_fops;  由于vdev.fops = msm_sensor_v4l2_subdev_fops = v4l2_subdev_fops; 故此处写成 fops = v4l2_subdev_ops;  
	v4l2_subdev_fops.compat_ioctl32 = msm_sensor_subdev_fops_ioctl;
	num;                                 
	cdev.ops = &v4l2_fops;
	dev->name = "/dev/v4l-subdev+num"
	vfl_type = VFL_TYPE_SUBDEV;
	dev->driver_data = v4l2_subdev;
};





####  msm_sensor_init.c  ####
static int __init msm_sensor_init_module(void);
主要用到__msm_sd_register_subdev()函数完成以下设置。
实体：
v4l2_subdev{
	name = "msm_sensor_init";
	v4l2_dev = msm_v4l2_dev;
	ops = msm_sensor_init_subdev_ops;
	internal_ops = &msm_sensor_init_internal_ops;
	flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	entity.group_id = MSM_CAMERA_SUBDEV_SENSOR_INIT;
	entity.name = "/dev/v4l-subdev+num";
	entity.parent = msm_v4l2_dev->mdev;
	devnode = video_device;
};

节点：
这个/dev/v4l2-subdev是用于生成上一个/dev/v4l2-subdev节点的。
video_device{
	name = "msm_sensor_init";
	fops = &msm_sensor_init_v4l2_subdev_fops = &v4l2_subdev_fops;
	msm_sensor_init_v4l2_subdev_fops.compat_ioctl32 = msm_sensor_init_subdev_fops_ioctl;
	ioctl_ops = &g_msm_ioctl_ops;
	num;                                 
	cdev.ops = &v4l2_fops;
	dev->name = "/dev/v4l-subdev+num"
	vfl_type = VFL_TYPE_SUBDEV;
	dev->driver_data = v4l2_subdev;
};

#################  end  驱动架构分析         ############

/*HAL层执行ioctl后，内核中
cdev.ops.unlocked_ioctl(struct file *,unsigned int cmd , void *arg)
{
	struct video_device *vdev = video_devdata(filp);
	vdev.fops.unlocked_ioctl(struct file *, , );
}
vdev.fops.unlocked_ioctl(struct file *, unsigned int cmd, void *arg )
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_subdev *sd = vdev_to_v4l2_subdev(vdev);
	sd.core.ioctl(sd, )
}
sd.core.ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{

}
*/



msm_sensor_driver_create_i2c_v4l_subdev->msm_sensor_subdev_fops_ioctl->msm_sensor_subdev_do_ioctl->msm_sensor_subdev_ioctl




hsm_probe分析:
1，PSOC的I2C通信
hsm_CheckEngineType(dev, adapter)用于从PSOC读sensor地址，可以通过此函数检查PSOC的I2C通信。
2，SENSOR的I2C通信
	if(	((hsm_Isn5600(dev, client) != 0) &&
		(hsm_Isn3601(dev, client) != 0)&&
#if 0
		(hsm_IsIT5000(dev, client) != 0)&&
#endif
		(hsm_Isn6700(dev, client) != 0))  ) 
	{
		ret = -ENODEV;
		goto err_gpio;
	}
如果可以读到SENSOR的版本号JADE_VERSION(0X0700)或者JADE_VERSION_2(0X0701)
说明SENSOR的I2C通信没问题。
3，MIPI2的I2C通信测试
hsm_test_mipi



关于dtsi中"qcom,csid-sd-index"属性值的研究：
mct_shimlayer_module_sensor_init
module_sensor_init("sensor")
	mct_module_t *s_module = NULL;
	module_sensor_ctrl_t *module_ctrl = NULL;
	sensor_init_probe(module_ctrl)
		找到/dev/v4l2-subdev(MSM_CAMERA_SUBDEV_SENSOR_INIT)并且打开，文件描述符为sd_fd。
		sensor_init_eebin_probe(module_ctrl, sd_fd);
		sensor_init_xml_probe(module_ctrl, sd_fd)
	module_sensor_find_other_subdev
		module_sensor_set_sub_module_id

msm_sensor_config
	s_ctrl->sensordata->sensor_info->subdev_id[i];

server.c main() 
server_process_module_init
module_sensor_init("sensor")
	bin_ctl.cmd = EEPROM_BIN_GET_BIN_DATA;
	/*eebin_interface_control(module_ctrl->eebin_hdl, &bin_ctl). ignore.*/
	module_sensor_ctrl_t *module_ctrl;
	sensor_init_probe(module_ctrl)
	|	//sensor_init_eebin_probe(module_ctrl, sd_fd)  return false. ignore.
		/*
			Get data from XML file and libmmcamera_n5600lib.so, store it in slave_info, and pass it to s_ctrl in kernel,
	|		and make kernel build /dev/media, /dev/video, /dev/v4l-subdev,and get subdev_id[] from s_ctrl.
			Finally build a sensor bundle , store all these data to bundle.
		*/
	|	sensor_init_xml_probe(module_ctrl, sd_fd)
			//Get root pointer and document pointer of XML file
			sensor_xml_util_load_file("/data/misc/camera/camera_config.xml", &docPtr, &rootPtr, "CameraConfigurationRoot");
	|		//Get number of camera module configurations
			num_cam_config = sensor_xml_util_get_num_nodes(rootPtr, "CameraModuleConfig");
			xmlConfig.docPtr = docPtr;
	|		xmlConfig.configPtr = &camera_cfg;
			for (i = 0; i < num_cam_config; i++) {
				//Get the ptr of i-th node  of XML file.
	|			nodePtr = sensor_xml_util_get_node(rootPtr, "CameraModuleConfig", i);				
				xmlConfig.nodePtr = nodePtr;
				/*Get data of "CameraModuleConfig" node except "CSIInfo" node and "LensInfo" node. 
	|			SensorName, ActuatorName, EepromName, OisName, FlashName, ChromatixName,
				CameraID,ModesSupported,Position,MountAngel,SensorSlaveAddress, 
				and store these data in xmlConfig.configPtr,which actually points to camerea_cfg.*/
	|			sensor_xml_util_get_camera_probe_config(&xmlConfig, "CameraModuleConfig")//Ethan: cp data from camera_config.xml to camera_cfg;
				/*
					sensor_probe get data from libmmcamera_n5600lib.so, and copy these data and data got from XML file to slave_info,
					and execute an ioctl to make kernel build /dev/meida named "msm-sensor", /dev/video named "msm-sensor",
					/dev/v4l-subdev named "n5600",
					and 
	|			*/
				sensor_probe(module_ctrl, sd_fd, camera_cfg.sensor_name, NULL, &xmlConfig, FALSE, FALSE)
					/*Load sensor library /system/vendor/lib/libmmcamera_n5600.so which is built from n5600_lib.c.
					And assign &sensor_lib_ptr which is defined in n5600_lib.h to sensor_lib_params->sensor_lib_ptr which will be used in translate_sensor_slave_info. */
					sensor_load_library(camera_cfg.sensor_name, sensor_lib_params, NULL)
						sensor_lib_params->sensor_lib_ptr = &sensor_lib_ptr
	|				/*
						Copy data of camer_cfg and sensor_lib_ptr->sensor_slave_info which are got from XML file and libmmcamera_n5600.so to slave_info.
						
					*/
					translate_sensor_slave_info(slave_info, &sensor_lib_params->sensor_lib_ptr->sensor_slave_info, xmlConfig->configPtr, power_up_setting, power_down_setting)
					Assign slave_info->output_format.
	|				memset(&cfg, 0, sizeof(cfg))
					cfg.cfgtype = CFG_SINIT_PROBE;
					cfg.cfg.setting = slave_info;        where is slave_info data get??? I suppose that it get from n5600lib.c,which is based on the context in msm_sensor_driver_probe function.
					/*
						Pass slave_info data to s_ctrl in kernel, and get subdev_id[] data  from s_ctrl which get this data in dtsi.
	|					Be attention that subdev_id[SUB_MODULE_CSID] will be set again in the next step.
						
					*/
					if (ioctl(fd, VIDIOC_MSM_SENSOR_INIT_CFG, &cfg) < 0) {
						video_device.name = "msm_sensor_init"
	|					v4l2_subdev_fops.compat_ioctl32
							msm_sensor_init_subdev_ioctl
								case VIDIOC_MSM_SENSOR_INIT_CFG:
									msm_sensor_driver_cmd(s_init, arg);
									case CFG_SINIT_PROBE:
	|								msm_sensor_driver_probe(cfg->cfg.setting,    what does this function do???	
										msm_sensor_driver_create_i2c_v4l_subdev(s_ctrl);
											camera_init_v4l2(&client->dev, &session_id);
											msm_sd.sd.name = sensorname="n5600" from camera_config.xml
											msm_sd_register(&s_ctrl->msm_sd);
											msm_sensor_fill_sensor_info(s_ctrl, probed_info, entity_name);
					/*
	|				Get data of CSIInfo node and LensInfo node of XML file, and chromatix info, store these data in xmlConfig->configPtr.
					*/
					sensor_xml_util_get_camera_full_config(xmlConfig)
					/* CSID core is taken from xml config file */
					cfg.probed_info.subdev_id[SUB_MODULE_CSID] =
					xmlConfig->configPtr->camera_csi_params.csid_core;
	|				/*
						Copy data of xmlConfig->configPtr and sensor_lib_params to a module_sensor_bundle_info_t struct, and append this struct
						to module_ctrl->sensor_bundle list.
					*/
					sensor_create_sbundle(module_ctrl,
                        &cfg.probed_info,
    |                   cfg.entity_name,
                        xmlConfig->configPtr,
                        sensor_lib_params,
                        FALSE,
    |                    FALSE)
    					01-22 00:12:45.000   665   665 D mm-camera: <SENSOR><  LOW> 70: sensor_create_sbundle: sensor name n5600 session 2
						01-22 00:12:45.000   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[0] 2   //SENSOR
						01-22 00:12:45.000   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[1] 0   //CHROMATIX
						01-22 00:12:45.000   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[2] -1
						01-22 00:12:45.000   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[3] -1
						01-22 00:12:45.000   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[4] -1
						01-22 00:12:45.000   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[5] -1
						01-22 00:12:45.001   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[6] 1  //CSID
						01-22 00:12:45.001   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[7] -1
						01-22 00:12:45.001   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[8] 2  //CSIPHY
						01-22 00:12:45.001   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[9] -1
						01-22 00:12:45.001   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[10] -1
						01-22 00:12:45.001   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[11] -1
						01-22 00:12:45.001   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[12] -1
						01-22 00:12:45.001   665   665 D mm-camera: <SENSOR><  LOW> 78: sensor_create_sbundle: subdev_id[13] -1
						01-22 00:12:45.001   665   665 D mm-camera: <SENSOR><  LOW> 99: sensor_create_sbundle: sensor sd name v4l-subdev17
	/*
	The follow text is a reference to MSM8937_LINUX_ANDROID_CAMERA_OVERVIEW.pdf:
	MSM8937/MSM8917 supports up to three MIPI cameras.
	 Camera interface 0 offers a 4-lane interface (connects to CSID-0).
	 Camera interface 1 offers a 4-lane interface (connects to CSID-1 and CSID2).
	 Camera interface 1 is used in two combinations.
	 4-lane – When only two cameras are to be supported on the system
	 2/1 lane – When three cameras are to be supported on the system
	*/
	/*
    module_sensor_find_other_subdev:
		Each subdev has an entity whose name is also subdev's name, and all the subdev's entities are children of a media device named "msm-config".
		So, we can get all subdev from this media device.
		This function will traverse every subdev, and match it's subdev_id which got from kernel with subdev_id[] in sensor_bundle.
		If the two id are matched, copy subdev's name ,for example /dev/v4l-subdev3, which got from kernel, to sensor_bundle->sensor_sd_name[SUB_MODULE_MAX][MAX_SUBDEV_SIZE].
		And module_sensor_fill_sub_module_info() copy sensor_bundle->sensor_info->subdev_id[f,e,SUB_MODULE_CSID] and subdev's name to sensor_bundle->subdev_info[SUB_MODULE_CSID].intf_info[SUBDEV_INF_PRIMARY].subdev_id and sensor_sd_name.
		In other words, sensor_bundle's subdev_id[] got from dtsi and XML file assigns user one of subdev which shares the same driver.
		For example, there are three subdevs of CSI, and each CSI subdev have different subdev_id(assigned by <cell-index> in different dtsi),
		we can choose one of them by set dtsi and XML file.
	*/
	/*
		According to the text above, the boot logcat-log, and the msm8953-camera.dtsi, MSM8953 has 3 CSID interfaces, which could connect with 3 MIPI
		cameras. System will build 3 /dev/subdev named "msm-csid" to correspond to 3 CSID interface by subdev ID. For example, subdev id 0 correspond to
		CSID-0 interface.
		So we should assign  csid core a correct value from 0 to 2 in XML file according to hardware connection.
	*/
	/*
		subdev id起到类似于次设备号的作用.该函数的作用是寻找与XML文件和dtsi文件中设定的设备号一致的字符设备,并将
		该字符设备的名字存储,以方便后面用.
		
	*/
	module_sensor_find_other_subdev(module_ctrl)
	 /* Init sensor modules */
	/*
		module_ctrl->sensor_bundle is a tree in which a node is a sensor_bundle, or a camera.
		mct_list_traverse() function traverse all node in the tree, and execute traverse(mct_list->data, user_data) function in each node.
		mct_list->data points to a sensor_bundle.
		In other word, mct_list_traverse() function execute traverse(node->data, user_data) in each node.
	*/
	/*
		module_sensors_subinit() assign subdev_id which equal SUB_MODULE_ACTUATOR, SUB_MODULE_EEPROM, SUB_MODULE_LED_FLASH, SUB_MODULE_STROBE_FLASH, SUB_MODULE_OIS
		-1 .And assign s_bundle->module_sensor_params[i]->func_tbl.open,process,and close a function address.
	*/
	mct_list_traverse(module_ctrl->sensor_bundle, module_sensors_subinit, NULL);
		static int32_t (*sub_module_init[SUB_MODULE_MAX])(sensor_func_tbl_t *) = {
		  [SUB_MODULE_SENSOR]       = sensor_sub_module_init,
		  [SUB_MODULE_CHROMATIX]    = chromatix_sub_module_init,
		  [SUB_MODULE_ACTUATOR]     = actuator_sub_module_init,
		  [SUB_MODULE_EEPROM]       = eeprom_sub_module_init,
		  [SUB_MODULE_LED_FLASH]    = led_flash_sub_module_init,
		  [SUB_MODULE_CSIPHY]       = csiphy_sub_module_init,
		  [SUB_MODULE_CSIPHY_3D]    = csiphy_sub_module_init,
		  [SUB_MODULE_CSID]         = csid_sub_module_init,
		  [SUB_MODULE_CSID_3D]      = csid_sub_module_init,
		  [SUB_MODULE_OIS]          = ois_sub_module_init,
		  [SUB_MODULE_EXT]          = external_sub_module_init
		}
		
		
	  /* Create ports based on CID info */
	mct_list_traverse(module_ctrl->sensor_bundle, port_sensor_create, s_module);
			


			
subdev_id[]数据流:
1, msm_sensor_driver_parse()中
	msm_sensor_get_sub_module_index(of_node, &s_ctrl->sensordata->sensor_info)从dtsi中获取
	eeprom-src,actuator-src,ois-src,,csiphy-sd-index,csid-sd-index等值,
	获取存入s_ctrl的subdev_id数组中.
2, msm_sensor_driver_probe()中
	msm_sensor_fill_actuator_subdevid_by_name()之类函数从dtsi中重新获取
	eeprom-src,actuator-src,ois-src(注意:没有重新获取csiphy-sd-index,csid-sd-index)的值,存入s_ctrl的subdev_id数组中.
	紧接着又通过msm_sensor_fill_sensor_info(s_ctrl, probed_info, entity_name)函数存入probed_info中,返回用户层sensor_probe()
3,在sensor_probe()函数中又进行了这样的操作.
	/* CSID core is taken from xml config file */
	cfg.probed_info.subdev_id[SUB_MODULE_CSID] =
	  xmlConfig->configPtr->camera_csi_params.csid_core;

mount_angle, position这两个值在xml和dtsi中都有,那最终存放在s_ctrl中的是哪个?
答:由sensor_probe()函数中translate_sensor_slave_info可知,是用的xml中的数据.msm_sensor_fill_slave_info_init_params
	将数据重新填入s_ctrl中.

执行完msm_sensor_driver_probe后s_ctrl和slave_info信息对比
执行完msm_sensor_driver_probe后,s_ctrl和上层slave_info重合的信息是一致的,只有subdev_id[SUB_MODULE_CSID ],上层slave_info会重新从XML文件获取,
而s_ctrl中依然保留从dtsi中获取的subdev_id[SUB_MODULE_CSID ].
那s_ctrl中的subdev_id[SUB_MODULE_CSID ]有何用呢?为什么改动下就报错?上层的subdev_id[SUB_MODULE_CSID ]又有何用?
msm_sensor_driver_probe
s_ctrl->sensordata->power_info   
s_ctrl->sensordata->slave_info->sensor_slave_addr = slave_info->slave_addr;
s_ctrl->sensordata->slave_info->sensor_id_reg_addr =slave_info->sensor_id_info.sensor_id_reg_addr;
s_ctrl->sensordata->slave_info->sensor_id = slave_info->sensor_id_info.sensor_id;
s_ctrl->sensordata->slave_info->sensor_id_mask = slave_info->sensor_id_info.sensor_id_mask;
s_ctrl->sensordata->sensor_name = slave_info->sensor_name;
s_ctrl->sensordata->eeprom_name = slave_info->eeprom_name;
s_ctrl->sensordata->actuator_name = slave_info->actuator_name;
s_ctrl->sensordata->ois_name = slave_info->ois_name;
s_ctrl->sensordata->flash_name = slave_info->flash_name;
s_ctrl->sensordata->sensor_info->sensor_mount_angle
s_ctrl->sensordata->sensor_info->position
s_ctrl->sensordata->sensor_info->modes_supported.
s_ctrl->sensordata->cam_slave_info = slave_info;
s_ctrl->sensor_i2c_client->addr_type = slave_info->addr_type;



s_ctrl->sensordata->subdev_id[EERPROM]
s_ctrl->sensordata->subdev_id[OIS]                  
s_ctrl->sensordata->subdev_id[ACTUATOR]                   from dtsi again, but emtpy.
s_ctrl->sensordata->subdev_id[FLASH]


msm_sensor_fill_sensor_info(s_ctrl, probed_info, entity_name);
probed_info->session_id = s_ctrl->sensordata->sensor_info->session_id; from dtsi??? yes
s_ctrl->sensordata->sensor_info->subdev_id[SUB_MODULE_SENSOR] =
	s_ctrl->sensordata->sensor_info->session_id;
for (i = 0; i < SUB_MODULE_MAX; i++) {
	sensor_info->subdev_id[i] =
		s_ctrl->sensordata->sensor_info->subdev_id[i];
	sensor_info->subdev_intf[i] =
		s_ctrl->sensordata->sensor_info->subdev_intf[i];
}

strlcpy(entity_name, s_ctrl->msm_sd.sd.entity.name, MAX_SENSOR_NAME);
probed_info的其他信息都是上层的slave_info的信息.


msm_sensor_get_sub_module_index
sensor_info->subdev_id[SUB_MODULE_CSIPHY ]   dtsi
sensor_info->subdev_id[SUB_MODULE_CSID ] 










参考高通手册interface部分，研究CSI PHY，以及研究软件框架
