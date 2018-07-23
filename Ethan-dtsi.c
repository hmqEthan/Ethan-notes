/*
 * Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "msm8953-pinctrl.dtsi"
#include "msm8953-camera-sensor-qrd.dtsi"
#include "msm8953-mdss-panels.dtsi"

&soc {
	gpio_keys {
		compatible = "gpio-keys";
		input-name = "gpio-keys";
		pinctrl-names = "tlmm_gpio_key_active","tlmm_gpio_key_suspend";
		pinctrl-0 = <&gpio_key_active>;
		pinctrl-1 = <&gpio_key_suspend>;

		vol_up {
			label = "volume_up";
			gpios = <&tlmm 85 0x1>;
			linux,input-type = <1>;
			linux,code = <115>;
			debounce-interval = <15>;
		};
		/*lidan modified start*/
		vol_down {
			label = "volume_down";
			gpios = <&tlmm 86 0x1>;
			linux,input-type = <1>;
			linux,code = <114>;
			gpio-key,wakeup;
			debounce-interval = <15>;
		};
		home_key {
			label = "home_key";
			gpios = <&tlmm 87 0x1>;
			linux,input-type = <1>;
			linux,code = <102>;
		};
		/*lidan modified end*/
	};

	i2c@78b7000 { /* BLSP1 QUP3 */
	/*	status = "okay";
		synaptics@4b {
			compatible = "synaptics,dsx-i2c";
			reg = <0x4b>;
			interrupt-parent = <&tlmm>;
			interrupts = <65 0x2008>;
			vdd_ana-supply = <&vdd_vreg>;
			vcc_i2c-supply = <&pm8953_l6>;
			synaptics,pwr-reg-name = "vdd_ana";
			synaptics,bus-reg-name = "vcc_i2c";
			synaptics,irq-gpio = <&tlmm 65 0x2008>;
			synaptics,irq-on-state = <0>;
			synaptics,irq-flags = <0x2008>;
			synaptics,power-delay-ms = <200>;
			synaptics,reset-delay-ms = <200>;
			synaptics,max-y-for-2d = <1919>;
			synaptics,cap-button-codes = <139 158 172>;
			synaptics,vir-button-codes = <139 180 2000 320 160
						      158 540 2000 320 160
						      172 900 2000 320 160>;
			synaptics,resume-in-workqueue;
			//Underlying clocks used by secure touch 
			clock-names = "iface_clk", "core_clk";
			clocks = <&clock_gcc clk_gcc_blsp1_ahb_clk>,
				<&clock_gcc clk_gcc_blsp1_qup3_i2c_apps_clk>;
		};
	*/
	};

	vdd_vreg: vdd_vreg {
		compatible = "regulator-fixed";
		status = "ok";
		regulator-name = "vdd_vreg";
	};
};



&spmi_bus {
	qcom,pmi8950@2 {
		qcom,leds@a100 {
			status = "okay";
			qcom,led_mpp_2 {
				label = "mpp";
				linux,name = "green";
				linux,default-trigger = "none";
				qcom,default-state = "off";
				qcom,max-current = <40>;
				qcom,current-setting = <5>;
				qcom,id = <6>;
				qcom,mode = "manual";
				qcom,source-sel = <1>;
				qcom,mode-ctrl = <0x60>;
			};
		};
	};
};

/ {
	qrd_batterydata: qcom,battery-data {
		qcom,batt-id-range-pct = <15>;
		#include "batterydata-qrd-sku1-4v4-2800mah.dtsi"
	};
};

/*&pm8953_typec {
	ss-mux-supply = <&pm8953_l6>;
	qcom,ssmux-gpio = <&tlmm 139 GPIO_ACTIVE_LOW>;
	pinctrl-names = "default";
	pinctrl-0 = <&typec_ssmux_config>;
};*/

&pmi8950_charger {
	qcom,battery-data = <&qrd_batterydata>;
	qcom,float-voltage-mv = <4250>;
	qcom,chg-led-sw-controls;
	qcom,chg-led-support;
	//qcom,external-typec;
	//qcom,typec-psy-name = "typec";
	qcom,thermal-mitigation = <3000 2500 2000 1500 1000 500 0>;
	qcom,fastchg-current-comp= <900>;
	status = "okay";
};

&pmi8950_fg {
	qcom,battery-data = <&qrd_batterydata>;
	qcom,cold-bat-decidegc = <(-150)>;
	qcom,cool-bat-decidegc = <0>;
	qcom,warm-bat-decidegc = <450>;
	qcom,hot-bat-decidegc = <550>;
	qcom,bad-battery-detection-enable;
	qcom,hold-soc-while-full;
	qcom,fg-cutoff-voltage-mv = <3400>;
	qcom,irq-volt-empty-mv = <3300>;
	qcom,fg-cc-cv-threshold-mv = <4200>;
};

&blsp1_uart0 {
	status = "ok";
	pinctrl-names = "default";
	pinctrl-0 = <&uart_console_active>;
};

&blsp1_uart2 {
	status = "ok";
	pinctrl-names = "default";
	pinctrl-0 = <&blsp2_uart2_active>;
};

&blsp2_uart2 {
	status = "ok";
	pinctrl-names = "default";
	pinctrl-0 = <&hsuart_active>;
};
&sdhc_1 {
	/* device core power supply */
	vdd-supply = <&pm8953_l8>;
	qcom,vdd-voltage-level = <2900000 2900000>;
	qcom,vdd-current-level = <200 570000>;

	/* device communication power supply */
	vdd-io-supply = <&pm8953_l5>;
	qcom,vdd-io-always-on;
	qcom,vdd-io-lpm-sup;
	qcom,vdd-io-voltage-level = <1800000 1800000>;
	qcom,vdd-io-current-level = <200 325000>;

	pinctrl-names = "active", "sleep";
	pinctrl-0 = <&sdc1_clk_on &sdc1_cmd_on &sdc1_data_on &sdc1_rclk_on>;
	pinctrl-1 = <&sdc1_clk_off &sdc1_cmd_off &sdc1_data_off &sdc1_rclk_off>;

	qcom,clk-rates = <400000 20000000 25000000 50000000 100000000 192000000
								384000000>;
	qcom,nonremovable;
	qcom,bus-speed-mode = "HS400_1p8v", "HS200_1p8v", "DDR_1p8v";

	status = "ok";
};

&sdhc_2 {
	/* device core power supply */
	vdd-supply = <&pm8953_l11>;
	qcom,vdd-voltage-level = <2950000 2950000>;
	qcom,vdd-current-level = <15000 800000>;

	/* device communication power supply */
	vdd-io-supply = <&pm8953_l12>;
	qcom,vdd-io-voltage-level = <1800000 2950000>;
	qcom,vdd-io-current-level = <200 22000>;

	pinctrl-names = "active", "sleep";
	pinctrl-0 = <&sdc2_clk_on &sdc2_cmd_on &sdc2_data_on &sdc2_cd_on>;
	pinctrl-1 = <&sdc2_clk_off &sdc2_cmd_off &sdc2_data_off &sdc2_cd_off>;

	#address-cells = <0>;
	interrupt-parent = <&sdhc_2>;
	interrupts = <0 1 2>;
	#interrupt-cells = <1>;
	interrupt-map-mask = <0xffffffff>;
	interrupt-map = <0 &intc 0 125 0
		1 &intc 0 221 0
		2 &tlmm 133 0>;
	interrupt-names = "hc_irq", "pwr_irq", "status_irq";
	cd-gpios = <&tlmm 133 0x1>;

	qcom,clk-rates = <400000 20000000 25000000 50000000 100000000
								200000000>;
	qcom,bus-speed-mode = "SDR12", "SDR25", "SDR50", "DDR50", "SDR104";

	status = "ok";
};

&i2c_5 { /* BLSP2 QUP1 (NFC) */
	nq@28 {   /*add by leo.tan for nfc pn548 2017-09-29 */
		compatible = "nxp,pn548";
		reg = <0x28>;
		nxp,irq-gpio = <&tlmm 59 0x00>;
		nxp,ven-gpio = <&tlmm 92 0x00>;
		nxp,firm-gpio = <&tlmm 38 0x00>;
		//nxp,clkreq-gpio = <&tlmm 3 0x00>;
		nxp,pwr-en = <&tlmm 47 0x00>;
		//nxp,rst-en = <&tlmm 38 0x00>;
		nfc_pvdd-supply = <&pm8953_l5>;
		//nxp,clk-src = "BBCLK2";
		//clocks = <&clock_gcc clk_bb_clk2>;
		//clock-names = "ref_clk";
	};
};

&i2c_2 {
	mipic: hsm,mipic@0e {					/* MIPI converter chip */
		compatible = "toshiba, tc358748";
		reg = <0x0e>;
		//gpios = <&tlmm 27 0x00>;			/* optional, reset pin */
		//hsm,gpio-mipi-reset = <0 1 0>;		/* index to gpios; init value; inverted */
	};

	qcom,camera@18{
	    cell-index=<0>;
		compatible = "hsm,n5600";	
		reg = <0x18>; //should contain i2c slave address of the device
		qcom,slave-id = <0x18 0xff 0x0700>;	//should contain i2c slave address, device id address,expected id read value and device id mask
		qcom,csiphy-sd-index = <1>;	//should contain csiphy instance that will used to receive sensor data
		qcom,csid-sd-index = <0>;	//should contain csid core instance that will used to receive sensor data
		/*qcom,mount-angle
		* The orientation of the camera image. The value is the angle that the
		* camera image needs to be rotated clockwise so it shows correctly on
		* the display in its natural orientation. It should be 0, 90, 180, or 270.
		*
		* For example, suppose a device has a naturally tall screen. The
		* back-facing camera sensor is mounted in landscape. You are looking at
		* the screen. If the top side of the camera sensor is aligned with the
		* right edge of the screen in natural orientation, the value should be
		* 90. If the top side of a front-facing camera sensor is aligned with
		* the right of the screen, the value should be 270.*
		*/
		qcom,mount-angle = <0>; //should contain the physical mount angle of the sensor on the target
		
		qcom,sensor-name = "n5600";	//should contain unique sensor name to differentiate from other sensor	
		cam_vdig-supply = <&pm8953_l2>;	//should contain regulator from which digital voltage is supplied
		cam_vio-supply = <&pm8953_l6>;	//should contain regulator from which IO voltage is supplied
		cam_vana-supply = <&pm8953_l22>;	//should contain regulator from which analog voltage is supplied
		//cam_vaf-supply = <&pm8953_l17>;	//should contain regulator from which AF voltage is supplied	
		qcom,cam-vreg-name = "cam_vdig", "cam_vio", "cam_vana";	//should contain names of all regulators needed by this sensor
		qcom,cam-vreg-min-voltage = <1200000 0 2800000>;	//should contain minimum voltage level in mcrovolts
																	//for regulators mentioned in qcom,cam-vreg-name property (in the same order)
		qcom,cam-vreg-max-voltage = <1200000 0 2800000>;	//should contain maximum voltage level in mcrovolts
																	//for regulators mentioned in qcom,cam-vreg-name property (in the same order)
		qcom,cam-vreg-op-mode = <200000 0 80000>;	//should contain the maximum current in microamps
															//required from the regulators mentioned in the qcom,cam-vreg-name property (in the same order)
		qcom,gpio-no-mux = <0>;	//should contain field to indicate whether gpio mux table is available
								// - 1 if gpio mux is not available, 0 otherwise
		pinctrl-names = "cam_default", "cam_suspend";
		pinctrl-0 = <&cam_sensor_mclk1_default
				&cam_sensor_front1_default>;
		pinctrl-1 = <&cam_sensor_mclk1_sleep
				&cam_sensor_front1_sleep>;
/*
		gpios = <&tlmm  0 0x00>,
		 		<&tlmm 9 0x00>, 
				<&tlmm 3 0x00>,
				<&tlmm  128 0x00>,
				<&tlmm 2 0x00>;
		hsm,gpio-power-enable = <0 0 0>;	// index to gpios; init value; inverted 
		hsm,gpio-aimer = <1 0 0>;
		hsm,gpio-illuminator = <2 0 0>;
		hsm,gpio-engine-reset = <3 0 0>;
		hsm,gpio-power-supply = <4 0 0>;
*/
		gpios = <&tlmm  27 0x00>,
		 		<&tlmm 128 0x00>, 
				<&tlmm 0 0x00>,
				<&tlmm  2 0x00>,
				<&tlmm 3 0x00>,
				<&tlmm 9 0x00>;
		hsm,gpio-engine-reset = <1 0 0>;
		hsm,gpio-power-enable = <2 0 0>;
		hsm,gpio-power-supply = <3 0 0>;
		hsm,gpio-aimer = <4 0 0>;
		hsm,gpio-illuminator = <5 0 0>;
		qcom,gpio-req-tbl-num = <0 1 2 3 4 5>;
		qcom,gpio-req-tbl-flags = <1 0 0 0 0 0>;
		qcom,gpio-req-tbl-label = "CAMIF_MCLK2",
			"SCANNER_RESET",
			"SCANNER_POWER_ENABLE",
			"SCANNER_POWER_SUPPLY",
			"SCANNER_AIMER",
			"SCANNER_ILLUMINATOR";


		qcom,csi-lane-assign = <0x4320>;	//should contain lane assignment value to map CSIPHY lanes to CSID lanes
		qcom,csi-lane-mask = <0x7>;			//should contain lane mask that specifies CSIPHY lanes to be enabled
		
		/*
		qcom,sensor-position : the follow context is copied from camera_config.xml
		Position of the sensor module. Valid values are:
		BACK, FRONT and BACK_AUX
	
		 enum camb_position_t {
			BACK_CAMERA_B,
			FRONT_CAMERA_B,
			AUX_CAMERA_B = 0x100,
			INVALID_CAMERA_B,
		};
		*/
		qcom,sensor-position = <1>;	//should contain the mount angle of the camera sensor
									//- 0 -> back camera - 1 -> front camera


		qcom,sensor-mode = <1>;		// should contain sensor mode supported  
									//- 0 -> back camera 2D 
									//- 1 -> front camera 2D 
									//- 2 -> back camera 3D  - 3 -> back camera int 3D
		qcom,cci-master = <1>;		//should contain i2c master id to be used for this camera sensor
		status = "ok";							// - 0 -> MASTER 0 		- 1 -> MASTER 1
		//qcom,is-2d;					
		hsm,mipic = <&mipic>;
		clocks = <&clock_gcc clk_mclk1_clk_src>,
			<&clock_gcc clk_gcc_camss_mclk1_clk>;
		clock-names = "cam_src_clk", "cam_clk";
		qcom,clock-rates = <24000000 0>;
	};
	
};
/*
&i2c_8 {  // aw9523 
	aw9523-keys@5B{
		compatible = "qcom,aw9523-keys";
		reg = <0x5B>;
		interrupt-parent = <&tlmm>;	
	  interrupts = <86 0x2>;
	  qcom,dis-gpio = <&tlmm 50 0x00>;
	  qcom,irq-gpio = <&tlmm 86 0x00>;
	  pinctrl-names = "pmx_aw9523_active","pmx_aw9523_suspend";
	  pinctrl-0 = <&aw9523_int_active>;
	  pinctrl-1 = <&aw9523_int_suspend>;
	  //irq_pull-supply = <&pm8917_15>;
   };
};*/
	  
&pm8953_gpios {
	/* GPIO 2 (NFC_CLK_REQ) */
	gpio@c100 {
		qcom,mode = <0>;
		qcom,output-type = <0>;
		qcom,pull = <0>;
		qcom,vin-sel = <2>;
		qcom,out-strength = <3>;
		qcom,src-sel = <0>;
		qcom,master-en = <1>;
		status = "okay";
	};
};

&i2c_3 {
	status = "ok";
};

&flash_led {
	/delete-node/ qcom,flash_1;
	/delete-node/ qcom,torch_1;
};

&led_flash0{
	qcom,flash-source = <&pmi8950_flash0>;
	qcom,torch-source = <&pmi8950_torch0>;
};

&pm8953_vadc {
	/delete-node/ chan@11;
};

&sdc2_cmd_on {
	config {
		drive-strength=<12>;
	};
};

&sdc2_data_on {
	config {
		drive-strength=<12>;
	};
};

