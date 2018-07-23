	
	待打印：
if (s_ctrl->func_tbl->sensor_match_id)打印下能否进入
s_ctrl = g_sctrl[slave_info->camera_id];打印下camera_id

内核层
msm_sensor_driver_parse()
rc = msm_sensor_driver_get_dt_data(s_ctrl);
	rc = of_property_read_u32(of_node, "cell-index", &cell_id);-->s_ctrl->id = cell_id
g_sctrl[s_ctrl->id] = s_ctrl;

g_sctrl用于存放msm_sensor_ctrl_t结构体数组。一个msm_sensor_ctrl_t存储一个sensor或者eeprom。

msm_sensor_driver_probe;{
	s_ctrl = g_sctrl[slave_info->camera_id];//slave_info->camera_id来自camera_config.xml文件中的camera_id
}
HAL层
g_camera_ctrl是一个mm_camera_ctrl_t全局变量，里面有mm_camera_obj_t *cam_obj[]结构体数组，用于存储camera对象。
get_num_of_cameras()中，获取所有名字为"msm-camera"的video实体，实体个数即为camera个数，存放在g_cam_ctrl.num_cam中
					if (s_ctrl->sensor_device_type == MSM_CAMERA_PLATFORM_DEVICE) {
		cci_client = s_ctrl->sensor_i2c_client->cci_client;

		/* Get CCI subdev */
		cci_client->cci_subdev = msm_cci_get_subdev();

		if (s_ctrl->is_secure)
			msm_camera_tz_i2c_register_sensor((void *)s_ctrl);

		/* Update CCI / I2C function table */
		if (!s_ctrl->sensor_i2c_client->i2c_func_tbl)
			s_ctrl->sensor_i2c_client->i2c_func_tbl =
				&msm_sensor_cci_func_tbl;
	} else {
		if (!s_ctrl->sensor_i2c_client->i2c_func_tbl) {
			CDBG("%s:%d\n", __func__, __LINE__);
			s_ctrl->sensor_i2c_client->i2c_func_tbl =
				&msm_sensor_qup_func_tbl;     //Ethan:will do this
		}
	}

错误1：


    5.363523] failed: cell-index rc -22      rc = of_property_read_u32(of_node, "cell-index", &cell_id);
rc = msm_sensor_driver_get_dt_data(s_ctrl);
[    5.366977] failed: rc -22<3>[    5.369685] hsm_imager 2-0018: Failed to msm_sensor_driver_parse, err = -22


//msm_sensor_driver_parse  failed
	//msm_sensor_driver_get_dt_data  failed
		//of_property_read_u32(of_node, "cell_index", &cell_id) failed
	//msm_sensor_init_default_params 并没有执行，那么该函数里面cci的设置就没有执行，到底在哪里执行的呢？
搜索发现msm_sensor_power_up中会有相应设置
msm_sensor_init_default_params

s_ctrl->func_tbl = &msm_sensor_func_tbl;
s_ctrl->func_tbl.sensor_power_up = msm_sensor_power_up,



rc = of_property_read_u32(of_node, "qcom,cci-master",
	&s_ctrl->cci_i2c_master);
CDBG("qcom,cci-master %d, rc %d", s_ctrl->cci_i2c_master, rc);
if (rc < 0) {
	/* Set default master 0 */
	s_ctrl->cci_i2c_master = MASTER_0;
	rc = 0;
}

cpp_probe()
	-->cpp_dev->msm_sd.sd.internal_ops = &msm_cpp_internal_ops;
		static const struct v4l2_subdev_internal_ops msm_cpp_internal_ops = {
			.open = cpp_open_node,
			.close = cpp_close_node,
		};
		cpp_open_node
			-->cpp_init_hardware
				-->msm_camera_clk_enable
					-->clk_prepare_enable(clk_ptr[i]);
						-->clk_enable
					-->clk->ops->enable(clk);
					-->	static struct rcg_clk camss_top_ahb_clk_src = {
						.cmd_rcgr_reg = CAMSS_TOP_AHB_CMD_RCGR,
						.set_rate = set_rate_mnd,
						.freq_tbl = ftbl_camss_top_ahb_clk_src,
						.current_freq = &rcg_dummy_freq,
						.base = &virt_bases[GCC_BASE],
						.c = {
							.dbg_name = "camss_top_ahb_clk_src",
							.ops = &clk_ops_rcg_mnd,
							VDD_DIG_FMAX_MAP2(LOW_SVS, 40000000, SVS_PLUS, 80000000),
							CLK_INIT(camss_top_ahb_clk_src.c),
							},
						};
							static struct branch_clk gcc_camss_cpp_ahb_clk = {
								.cbcr_reg = CAMSS_CPP_AHB_CBCR,
								.has_sibling = 1,
								.base = &virt_bases[GCC_BASE],
								.c = {
									.dbg_name = "gcc_camss_cpp_ahb_clk",
									.parent = &camss_top_ahb_clk_src.c,
									.ops = &clk_ops_branch,
									CLK_INIT(gcc_camss_cpp_ahb_clk.c),
									},
								};
						struct clk_ops clk_ops_rcg_mnd = {
								.enable = rcg_clk_enable,
							rcg_clk_enable

(*clk_ptr)[i] = devm_clk_get(dev, "camss_top_ahb_clk");//根据"camss_top_ahb_clk"，从dev中获取与之相应的指针。
	-->struct clk *devm_clk_get(dev, "camss_top_ahb_clk")
		-->clk = clk_get(dev, "camss_top_ahb_clk");
			-->clk = of_clk_get_by_name(dev->of_node, "camss_top_ahb_clk");	
					index = of_property_match_string(dev->of_node, "clock-names", "camss_top_ahb_clk"));	
					clk = of_clk_get(dev->of_node, index);
						；of_parse_phandle_with_args(dev->of_node, "clocks", "#clock-cells", 0,&clkspc);
						clk = of_clk_get_by_clkspec(&clkspec);






1）属性值是text string或者string list，用双引号表示。例如device_type = "memory"

2）属性值是32bit unsigned integers，用尖括号表示。例如#size-cells = <1>

3）属性值是binary data，用方括号表示。例如binary-property = [0x01 0x23 0x45 0x67]

memory device node是所有设备树文件的必备节点，它定义了系统物理内存的layout。
device_type属性定义了该node的设备类型，例如cpu、serial等。对于memory node，
其device_type必须等于memory。reg属性定义了访问该device node的地址信息，
该属性的值被解析成任意长度的（address，size）数组，具体用多长的数据来表示
address和size是在其parent node中定义（#address-cells和#size-cells）。
对于device node，reg描述了memory-mapped IO register的offset和length。
对于memory node，定义了该memory的起始地址和长度。


						/ { 
    #address-cells = <1>; 
    #size-cells = <1>; 
    chosen { }; 
    aliases { }; 
    memory { device_type = "memory"; reg = <0 0>; }; 
};
#address-cells 和 #size-cells的理解：
首先要理解reg = <0 0>，reg属性的属性值中，前一部分用于描述memory的起始地址，offset，
后一部分用于描述长度，即length。但如果reg=<0x01 0x02 0x03>，前面几个用于描述地址呢？
这就需要#address-cells和#size-cells属性了。这两个属性都定义在该节点的父节点中，
#表示个数，#address-cells的属性值1表示reg中前1个cell表示offset，#size-cells属性值1
表示reg中后1个cell表示length。
一个cell表示一个u32














#include "skeleton.dtsi"

/ { 
    compatible = "samsung,s3c24xx"; －－－－－－－－－－－－－－－－－－－（A） 
    interrupt-parent = <&intc>; －－－－－－－－－－－－－－－－－－－－－－（B）

    aliases { 
        pinctrl0 = &pinctrl_0; －－－－－－－－－－－－－－－－－－－－－－－－（C） 
    };

    intc:interrupt-controller@4a000000 { －－－－－－－－－－－－－－－－－－（D） 
        compatible = "samsung,s3c2410-irq"; 
        reg = <0x4a000000 0x100>; 
        interrupt-controller; 
        #interrupt-cells = <4>; 
    };

    serial@50000000 { －－－－－－－－－－－－－－－－－－－－－－（E）  
        compatible = "samsung,s3c2410-uart"; 
        reg = <0x50000000 0x4000>; 
        interrupts = <1 0 4 28>, <1 1 4 28>; 
        status = "disabled"; 
    };

    pinctrl_0: pinctrl@56000000 {－－－－－－－－－－－－－－－－－－（F） 
        reg = <0x56000000 0x1000>;

        wakeup-interrupt-controller { 
            compatible = "samsung,s3c2410-wakeup-eint"; 
            interrupts = <0 0 0 3>,
                     <0 0 1 3>,
                     <0 0 2 3>,
                     <0 0 3 3>,
                     <0 0 4 4>,
                     <0 0 5 4>;
        }; 
    };

…… 
};


intc是一个lable，标识了一个device node（在本例中是标识了
interrupt-controller@4a000000 这个device node）。实际上，interrupt-parent
属性值应该是是一个u32的整数值（这个整数值在Device Tree的范围内唯一识别了一个
device node，也就是phandle），不过，在dts文件中中，可以使用类似c语言的Labels 
and References机制。定义一个lable，唯一标识一个node或者property，后续可以使用
&来引用这个lable。DTC会将lable转换成u32的整数值放入到DTB中，用户层面就不再关心
具体转换的整数值了。
）具体各个HW block的interrupt source是如何物理的连接到interruptcontroller的呢？
在dts文件中是用interrupt-parent这个属性来标识的。且慢，这里定义interrupt-parent
属性的是root node，难道root node会产生中断到interrupt controller吗？当然不会，
只不过如果一个能够产生中断的device node没有定义interrupt-parent的话，其
interrupt-parent属性就是跟随parent node。因此，与其在所有的下游设备中定义
interrupt-parent，不如统一在root node中定义了。

问题：
1，既然dtsi中，i表示include，那么dts文件如何包含dtsi文件？如果引用的label不在本文件，会在其包含的dtsi中么？
2，父节点和子节点的关系如何表示？没见有子节点中有parent呢？
答：通过{}表示。
/ {  
    node1 {  
        a-string-property = "A string";  
        a-string-list-property = "first string", "second string";  
        a-byte-data-property = [0x01 0x23 0x34 0x56];  
        child-node1 {  
            first-child-property;  
            second-child-property = <1>;  
            a-string-property = "Hello, world";  
        };  
        child-node2 {  
        };  
    };  
    node2 {  
        an-empty-property;  
        a-cell-property = <1 2 3 4>; /* each number (cell) is a uint32 */  
        child-node1 {  
        };  m
    };  
}; dd

3，cell-index表示什么？

msm_sensor_driver_parse

rc = msm_sensor_driver_get_dt_data(s_ctrl);

msm_sensor_get_sub_module_index(of_node, &sensordata->sensor_info)

