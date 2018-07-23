static int allocate_shared_buffer(struct shared_buffer *buf, int ion_type);会利用/dev/ion在内核分配一块内存。
	那mapbuf做了什么？
	camera_open创建了socket，addchannel创建了线程，这两个东东什么时候会用到？
	为什么stream_size会有高度和宽度呢?
	/Ethan: memptr在allocatebuffer中不是已经映射过了？为什么还要清空？



	ADD_SET_PARAM_ENTRY_TO_BATCH(parm_buffer_ptr, CAM_INTF_PARM_HAL_VERSION, hal_version);	

#define ADD_SET_PARAM_ENTRY_TO_BATCH(TABLE_PTR, META_ID, DATA) \
    ((NULL != TABLE_PTR) ? ((TABLE_PTR->data.member_variable_##META_ID[ 0 ] = DATA), (TABLE_PTR->is_valid[META_ID] = 1), (0)) 
	((NULL != TABLE_PTR) ? ((parm_buffer_ptr->data.member_variable_##1[ 0 ] = hal_version), (parm_buffer_ptr->is_valid[1] = 1), (0))
	ADD_SET_PARAM_ENTRY_TO_BATCH(parm_buffer_ptr, CAM_INTF_META_STREAM_INFO, stream_config_info);
	 ((NULL != TABLE_PTR) ? ((parm_buffer_ptr->data.member_variable_##CAM_INTF_META_STREAM_INFO[ 0 ] = stream_config_info), (parm_buffer_ptr->is_valid[CAM_INTF_META_STREAM_INFO] = 1), (0)) 


static void *mm_camera_cmd_thread(void *data);

node = (mm_camera_cmdcb_t*)cam_queue_deq(&cmd_thread->cmd_queue);//Ethan: 提取一个节点的数据出来，并在链表中删除这个节点。
	   while (node != NULL) {//Ethan: 对这个节点进行分析处理，这个节点是哪里来的呢？貌似是另一个可以给该链表加节点并发送条件变量信号的地方。
		   switch (node->cmd_type) {


##########mm_camera_open分析###################


mm_camera_cmd_thread_launch(&my_obj->evt_thread,mm_camera_dispatch_app_event,(void *)my_obj);
该函数会启动一个线程，该线程用my_obj->evt_thread描述，该线程的结构是：
等待条件变量的状态改变，一旦改变evt_thread->cmd_sem中的条件变量的状态，就获取一个evt_thread->cmd_queue队列的节点，
并获取该节点的数据。cmd_queue结构体如下所示，该结构体描述了一个队列，cam_node_t 是一个链表，其结构体如下所示。当条件
变量变化时，线程取出队列中第一个节点的数据，删除该队列的这个节点，并根据该数据的cmd_type进行处理，如果cmd_type==MM_CAMERA_CMD_TYPE_FLUSH_QUEUE
调用回调函数mm_camera_dispatch_app_event根据该数据进行处理。
typedef struct {
    cam_node_t head; /* dummy head */
    uint32_t size;
    pthread_mutex_t lock;
} cam_queue_t;
typedef struct {
    struct cam_list list;
    void *data;
} cam_node_t;

snprintf(my_obj->evt_poll_thread.threadName, THREAD_NAME_SIZE, "CAM_evntPoll");
mm_camera_poll_thread_launch(&my_obj->evt_poll_thread, MM_CAMERA_POLL_TYPE_EVT);
该函数启动一个线程，该线程用my_obj->evt_poll_thread描述，该线程的结构是用poll函数监听一个管道文件描述符的读事件。
static void *mm_camera_poll_thread(void *data);
{
    poll_cb->poll_fds[poll_cb->num_fds++].fd = poll_cb->pfds[0];
	return mm_camera_poll_fn(poll_cb)
}
static void *mm_camera_poll_fn(mm_camera_poll_thread_t *poll_cb);
{
	
	for(i = 0; i < poll_cb->num_fds; i++) {
            poll_cb->poll_fds[i].events = POLLIN|POLLRDNORM|POLLPRI;
         }
	rc = poll(poll_cb->poll_fds, poll_cb->num_fds, poll_cb->timeoutms);
	if(rc > 0) {
            if ((poll_cb->poll_fds[0].revents & POLLIN) &&
                (poll_cb->poll_fds[0].revents & POLLRDNORM)) {
                mm_camera_poll_proc_pipe(poll_cb);
            } else {
                for(i=1; i<poll_cb->num_fds; i++) {
                    /* Checking for ctrl events */
                    if ((poll_cb->poll_type == MM_CAMERA_POLL_TYPE_EVT) &&
                        (poll_cb->poll_fds[i].revents & POLLPRI)) {
                       
                        if (NULL != poll_cb->poll_entries[i-1].notify_cb) {
                            poll_cb->poll_entries[i-1].notify_cb(poll_cb->poll_entries[i-1].user_data);
                        }
                    }

                    if ((MM_CAMERA_POLL_TYPE_DATA == poll_cb->poll_type) &&
                        (poll_cb->poll_fds[i].revents & POLLIN) &&
                        (poll_cb->poll_fds[i].revents & POLLRDNORM)) {
                        LOGD("mm_stream_data_notify\n");
                        if (NULL != poll_cb->poll_entries[i-1].notify_cb) {
                            poll_cb->poll_entries[i-1].notify_cb(poll_cb->poll_entries[i-1].user_data);
                        }
                    }
                }
            }
        } else {
            /* in error case sleep 10 us and then continue. hard coded here */
            usleep(10);
            continue;
        }
}
poll_cb是mm_camera_poll_thread_t结构，poll_fds是该结构体的一个元素，用于保存所有需要监控的fd，根据本程序，这个fd=pfds[0];
且num_fds=1，pfd[2]也是结构体的一个元素，是用pipe创建的一个管道文件描述符。
	

int32_t mm_camera_evt_sub(mm_camera_obj_t * my_obj,uint8_t reg_flag);
{
该函数首先通过/dev/video进行ioctl("/dev/video",VIDIOC_SUBSCRIBE_EVENT,&sub);其中，
sub.type = MSM_CAMERA_V4L2_EVENT_TYPE;
sub.id = MSM_CAMERA_MSM_NOTIFY;
在内核中会调用到：
v4l2_event_subscribe(&sp->fh, sub, 5, NULL)：
found_ev = v4l2_event_subscribed(fh, sub->type, sub->id);
	if (!found_ev)
		list_add(&sev->list, &fh->subscribed);
注册事件只是向fh->subscribed链表中添加一个节点么？

/* add evt fd to polling thread when subscribe the first event */
	
	rc = mm_camera_poll_thread_add_poll_fd(&my_obj->evt_poll_thread,
										   my_obj->my_hdl,
										   my_obj->ctrl_fd,
										   mm_camera_event_notify,
										   (void*)my_obj,
										   mm_camera_sync_call);

该函数会将ctrl_fd,my_hdl,mm_camera_event_notify, (void*)my_obj,分别赋予my_obj->evt_poll_thread.poll_entries[idx].fd,handler,notify_cb,user_data;
并通过向my_obj->evt_poll_thread.pfds[1]，向mm_camera_poll_thread_launch创建的进程发送命令MM_CAMERA_PIPE_CMD_POLL_ENTRIES_UPDATED。该线程收到命令后，通过读取
pfd[0]获取命令，根据该命令，什么都没处理。按理说，收到该命令后，应该将poll_entries[idx]的fd(即ctrl_fd)加入poll监听数组poll_fds[]中，
MM_CAMERA_PIPE_CMD_POLL_ENTRIES_UPDATED_ASYNC命令是这样处理的。

***注：其实my_obj->evt_poll_thread.pfd[0]和pfd[1]是一个与该线程连接的桥梁，其他线程通过对pfd[1]的写操作，触发线程的poll
，也可以由此向my_obj->evt_poll_thread.poll_fds[]数组中添加需要监控的fd，本例中将my_obj->ctrl_fd(open("/dev/video"))添加了进去，
当内核对该fd有写操作时，线程会响应。难道这就是event机制？但是内核如何向fd写操作呢？poll_entries[idx].notify_cb又何时
调用呢？
答：这些问题需要研究poll可以检测的fd的事件类型。
答：由mm_camera_poll_fn函数可知，当有事件产生时，首先判断pfd[0]，并执行mm_camera_poll_proc_pipe(poll_cb)函数，像上文说的那样
pfd[0]和pfd[1]是一个桥梁。再次判断pfd数组的其他fd，这里面是我们添加的fd，并调用响应entries的notify_cb函数。

}



idChannel = pCamera->ops->add_channel(pCamera->camera_handle, NULL, 0, NULL)
	-->mm_camera_intf_add_channel(pCamera->camera_handle, NULL, 0, NULL)
		-->mm_camera_add_channel(my_obj, NULL, 0, NULL)
		1，在MM_CAMERA_CHANNEL_MAX个通道中找一个没有使用的（MM_CHANNEL_STATE_NOTUSED）通道ch_obj;
		2，给ch_obj赋值，state（MM_CHANNEL_STATE_STOPPED），my_hdl(ch_hdl)，cam_obj（my_obj），sessionid（my_obj->sessionid）
			-->mm_channel_init(ch_obj, NULL, 0, NULL)
			为ch_obj->poll_thread[0].ThreadName赋值（"CAM_dataPoll"）
				-->mm_camera_poll_thread_launch(&my_obj->poll_thread[0],MM_CAMERA_POLL_TYPE_DATA);

idStream = pCamera->ops->add_stream(pCamera->camera_handle, idChannel);
	--> mm_camera_intf_add_stream(pCamera->camera_handle,idChannel);
		-->stream_id = mm_camera_add_stream(my_obj, ch_id);
			-->mm_channel_fsm_fn(ch_obj, MM_CHANNEL_EVT_ADD_STREAM, NULL, (void *)&s_hdl);
				-->mm_channel_fsm_fn_stopped(ch_obj, MM_CHANNEL_EVT_ADD_STREAM, NULL, (void *)&s_hdl);
					-->s_hdl = mm_channel_add_stream(ch_obj);*((uint32_t*)out_val) = s_hdl;
           			 	stream_obj->state = MM_STREAM_STATE_INITED;
						   -->mm_stream_fsm_fn(stream_obj, MM_STREAM_EVT_ACQUIRE, NULL, NULL);
						   		--> mm_stream_fsm_inited(stream_obj, MM_STREAM_EVT_ACQUIRE, NULL, NULL);
									stream_obj->fd = open("/dev/video", O_RDWR | O_NONBLOCK);
									|-->mm_stream_set_ext_mode(stream_obj);
									|		ioctl(stream_obj->fd, VIDIOC_S_PARM, &s_parm);
												--> what is done in kernel?
									|		stream_obj->server_stream_id = s_parm.parm.capture.extendedmode;
											#ifndef DEAMON_PRESENT 
									|		/*cam_shim_packet_t *shim_cmd;
	        								cam_shim_cmd_data shim_cmd_data;
									|        mm_camera_obj_t *cam_obj = stream_obj->ch_obj->cam_obj;
									
									|        memset(&shim_cmd_data, 0, sizeof(shim_cmd_data));
									        shim_cmd_data.command = MSM_CAMERA_PRIV_NEW_STREAM;
									|        shim_cmd_data.stream_id = stream_obj->server_stream_id;
									        shim_cmd_data.value = NULL;
									|        shim_cmd = mm_camera_create_shim_cmd_packet(CAM_SHIM_SET_PARM,
									                cam_obj->sessionid, &shim_cmd_data);
									|        mm_camera_module_send_cmd(shim_cmd);
											-->mct_shimlayer_process_event(shim_cmd)
									|			do nothing
												/*-->mct_shimlayer_reg_buffer
									|				-->mct_controller_proc_serv_msg(&serv_msg) */
									|		#endif*/
								    stream_obj->state= MM_STREAM_STATE_ACQUIRED	
						
/*
	   map param buffer  start
	*/
 /* mapping getparm buf */
 /* alloc ion mem for getparm buf */
 memset(&g_parm_buf, 0, sizeof(g_parm_buf));
 g_parm_buf.cbsize = sizeof(parm_buffer_t);
 allocate_shared_buffer(&g_parm_buf, 0);
	ion_device_handle = open(ION_DEVICE, O_RDONLY);
 		

pCamera->ops->map_buf(pCamera->camera_handle, CAM_MAPPING_BUF_TYPE_PARM_BUF, g_parm_buf.fd, g_parm_buf.cbsize, NULL);
-->mm_camera_intf_map_buf(pCamera->camera_handle, CAM_MAPPING_BUF_TYPE_PARM_BUF, g_parm_buf.fd, g_parm_buf.cbsize, NULL)
	-->mm_camera_map_buf(my_obj, CAM_MAPPING_BUF_TYPE_PARM_BUF,  g_parm_buf.fd, g_parm_buf.cbsize, NULL);
			cam_sock_packet_t packet;
		    packet.msg_type = CAM_MAPPING_TYPE_FD_MAPPING;
		    packet.payload.buf_map.type = CAM_MAPPING_BUF_TYPE_PARM_BUF;
		    packet.payload.buf_map.fd = g_parm_buf.fd;
		    packet.payload.buf_map.size = g_parm_buf.cbsize;
		    packet.payload.buf_map.buffer = NULL;
			mm_camera_util_sendmsg(my_obj, &packet, sizeof(cam_sock_packet_t), g_parm_buf.fd);									
				if(mm_camera_socket_sendmsg(my_obj->ds_fd, &packet, sizeof(cam_sock_packet_t), g_parm_buf.fd) > 0)
       			 /* wait for event that mapping/unmapping is done */
        		mm_camera_util_wait_for_event(my_obj, CAM_EVENT_TYPE_MAP_UNMAP_DONE, &status);
        		if (MSM_CAMERA_STATUS_SUCCESS == status) {
          			  rc = 0;
 



stream是什么鬼？在内核中创建的是什么东东？
//msm_create_stream(event_data->session_id,
		event_data->stream_id, &sp->vb2_q);，session_id和stream_id分别是什么东东？
		//sp->vb2_q是哪里来的？
创建一个msm_stream结构体，将其加入msm_session结构体的stream_q队列，就算是创建了一个stream?
//struct msm_queue_head *msm_session_q;
//session = msm_queue_find(msm_session_q, struct msm_session,list, __msm_queue_find_session, &session_id);
找到session队列msm_session_q中session_id与指定的session_id一致的session

msm_post_event做了什么？
	--> v4l2_event_queue(vdev, event)

mm_stream_set_ext_mode

/* Only bundle streams that belong to the channel */
            if(!(s_objs[i]->stream_info->noFrameExpected)) {

这是什么意思？


mct_shimlayer_module_sensor_init
	-->tmp=module_sensor_init("sensor")
		-->mct_module_t *s_module = mct_module_create("sensor");



mct_shimlayer_process_module_init

shim_ops_tbl->mm_camera_shim_open_session = mct_shimlayer_start_session;
shim_ops_tbl->mm_camera_shim_close_session = mct_shimlayer_stop_session;
shim_ops_tbl->mm_camera_shim_send_cmd = mct_shimlayer_process_event;



SimpleBarcodeReader(void)
		decConnect(void)
			CHalInterface
				InitHal()
					InitHWLayer
						camera_init
				InitImagerTester();
					bool HwlBase :: ImagerPowerOn()	
							gpio_power_on
							mipi_init
								toshiba_init
									gp_Hwl->ImagerPowerUp
										gpio_set_value(HSM_GPIO_ENGINE_RESET, 0); // take the engine out of reset
										gpio_set_value(HSM_GPIO_POWER_ENABLE, 1); // raise power enable
									toshiba_read_reg(TOSHIBA_CHIPID_REG, &read_buffer, 1)
						bool HwlBase :: ImagerPowerUp(void)
				InitImageDebug(GetHal()->GetScanWidth(),GetHal()->GetScanHeight());
}







