/* Copyright (c) 2016, The Linux Foundation. All rights reserved.
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

/*!@file: DWC_ETH_QOS_plt_init.c
 * @brief: Driver functions.
 */

#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/pm.h>
#include <linux/pm_wakeup.h>
#include <linux/sched.h>
#include <linux/pm_qos.h>
#include <linux/pm_runtime.h>
#include <linux/esoc_client.h>
#include <linux/pinctrl/consumer.h>
#include "DWC_ETH_QOS_yheader.h"

#ifdef CONFIG_PCI_MSM
#include <linux/msm_pcie.h>
#else
#include <mach/msm_pcie.h>
#endif

#define NTN_VREG_CORE_NAME 	"vdd-ntn-core"
#define NTN_VREG_PHY_NAME 	"vdd-ntn-phy"
#define NTN_VREG_HSIC_NAME 	"vdd-ntn-hsic"
#define NTN_VREG_PCI_NAME 	"vdd-ntn-pci"
#define NTN_VREG_IO_NAME 	"vdd-ntn-io"
#define NTN_VREG_CORE_MIN	1100000
#define NTN_VREG_CORE_MAX	1100000
#define NTN_VREG_PHY_MIN	2500000
#define NTN_VREG_PHY_MAX	2500000
#define NTN_VREG_HSIC_MIN	1200000
#define NTN_VREG_HSIC_MAX	1200000
#define NTN_VREG_PCI_MIN	1800000
#define NTN_VREG_PCI_MAX	1800000
#define NTN_VREG_IO_MIN		1800000
#define NTN_VREG_IO_MAX		1800000
#define NTN_RESET_GPIO_NAME	"ntn-rst-gpio"
#define NTN_PWR_GPIO_NAME	"ntn-supply-enable-gpio"

extern int DWC_ETH_QOS_init_module(void);
extern void DWC_ETH_QOS_exit_module(void);

int resx_gpio_num = -1;
int supply_gpio_num = -1;
struct regulator *ntn_reg_hsic = NULL;
struct regulator *ntn_reg_pci = NULL;
struct regulator *ntn_reg_io = NULL;
struct regulator *ntn_reg_core = NULL;
struct regulator *ntn_reg_phy = NULL;

/*!
* \brief Initialize and turn on regulators
*
* \details Parse the device tree for regulator nodes and
* turn them on. Missing nodes are non fatal.
*
* \param[in] idev - struct device from PCI device
*
* \return 0 on success. Negative error code on failure
*/
static int DWC_ETH_QOS_init_regulators(struct device *idev)
{
	int ret = 0;

	if (!idev)
	{
		return -EINVAL;
	}

	/* Configure power rails Neutrino*/
	if (of_get_property(idev->of_node,NTN_VREG_HSIC_NAME"-supply", NULL))
	{
		ntn_reg_hsic = regulator_get(idev, NTN_VREG_HSIC_NAME);
		if (IS_ERR(ntn_reg_hsic))
		{
			ret = PTR_ERR(ntn_reg_hsic);

			if (ret == -EPROBE_DEFER)
			{
				NMSGPR_ALERT( "%s: ntn_reg_hsic probe deferred!\n", __func__);
			}
			else
			{
				NMSGPR_ALERT( "%s: Get ntn_reg_hsic failed!\n", __func__);
			}
			goto exit_error;
		}
		else
		{
			ret = regulator_set_voltage(ntn_reg_hsic, NTN_VREG_HSIC_MIN, NTN_VREG_HSIC_MAX);
			if (ret)
			{
				NMSGPR_ALERT( "%s: Set ntn_reg_hsic failed!\n", __func__);
				goto exit_error;
			}
			else
			{
				ret = regulator_enable(ntn_reg_hsic);
				if (ret)
				{
					NMSGPR_ALERT( "%s: Enable ntn_reg_hsic failed!\n", __func__);
					goto exit_error;
				}
			}
		}
	}
	else
	{
		NMSGPR_ALERT( "%s: power supply %s not found!\n", __func__,NTN_VREG_HSIC_NAME);
	}

	if (of_get_property(idev->of_node,NTN_VREG_PCI_NAME"-supply", NULL))
	{
		ntn_reg_pci = regulator_get(idev, NTN_VREG_PCI_NAME);
		if (IS_ERR(ntn_reg_pci))
		{
			ret = PTR_ERR(ntn_reg_pci);

			if (ret == -EPROBE_DEFER)
			{
				NMSGPR_ALERT( "%s: ntn_reg_pci probe deferred!\n", __func__);
			}
			else
			{
				NMSGPR_ALERT( "%s: Get ntn_reg_pci failed!\n", __func__);
			}
			goto exit_error;
		}
		else
		{
			ret = regulator_set_voltage(ntn_reg_pci, NTN_VREG_PCI_MIN, NTN_VREG_PCI_MAX);
			if (ret)
			{
				NMSGPR_ALERT( "%s: Set ntn_reg_pci failed!\n", __func__);
				goto exit_error;
			}
			else
			{
				ret = regulator_enable(ntn_reg_pci);
				if (ret)
				{
					NMSGPR_ALERT( "%s: Enable ntn_reg_pci failed!\n", __func__);
					goto exit_error;
				}
			}
		}
	}
	else
	{
		NMSGPR_ALERT( "%s: power supply %s not found!\n", __func__,NTN_VREG_PCI_NAME);
	}

	if (of_get_property(idev->of_node,NTN_VREG_IO_NAME"-supply", NULL))
	{
		ntn_reg_io = regulator_get(idev, NTN_VREG_IO_NAME);
		if (IS_ERR(ntn_reg_io))
		{
			ret = PTR_ERR(ntn_reg_io);

			if (ret == -EPROBE_DEFER)
			{
				NMSGPR_ALERT( "%s: ntn_reg_io probe deferred!\n", __func__);
			}
			else
			{
				NMSGPR_ALERT( "%s: Get ntn_reg_io failed!\n", __func__);
			}
			goto exit_error;
		}
		else
		{
			ret = regulator_set_voltage(ntn_reg_io, NTN_VREG_IO_MIN, NTN_VREG_IO_MAX);
			if (ret)
			{
				NMSGPR_ALERT( "%s: Set ntn_reg_io failed!\n", __func__);
				goto exit_error;
			}
			else
			{
				ret = regulator_enable(ntn_reg_io);
				if (ret)
				{
					NMSGPR_ALERT( "%s: Enable ntn_reg_io failed!\n", __func__);
					goto exit_error;
				}
			}
		}
	}
	else
	{
		NMSGPR_ALERT( "%s: power supply %s not found!\n", __func__,NTN_VREG_IO_NAME);
	}

	if (of_get_property(idev->of_node,NTN_VREG_CORE_NAME"-supply", NULL))
	{
		ntn_reg_core = regulator_get(idev, NTN_VREG_CORE_NAME);
		if (IS_ERR(ntn_reg_core))
		{
			ret = PTR_ERR(ntn_reg_core);

			if (ret == -EPROBE_DEFER)
			{
				NMSGPR_ALERT( "%s: ntn_reg_core probe deferred!\n", __func__);
			}
			else
			{
				NMSGPR_ALERT( "%s: Get ntn_reg_core failed!\n", __func__);
			}
			goto exit_error;
		}
		else
		{
			ret = regulator_set_voltage(ntn_reg_core, NTN_VREG_CORE_MIN, NTN_VREG_CORE_MAX);
			if (ret)
			{
				NMSGPR_ALERT( "%s: Set ntn_reg_core failed!\n", __func__);
				goto exit_error;
			}
			else
			{
				ret = regulator_enable(ntn_reg_core);
				if (ret)
				{
					NMSGPR_ALERT( "%s: Enable ntn_reg_core failed!\n", __func__);
					goto exit_error;
				}
			}
		}
	}
	else
	{
		NMSGPR_ALERT( "%s: power supply %s not found!\n", __func__,NTN_VREG_CORE_NAME);
	}

	if (of_get_property(idev->of_node,NTN_VREG_PHY_NAME"-supply", NULL))
	{
		ntn_reg_phy = regulator_get(idev, NTN_VREG_PHY_NAME);
		if (IS_ERR(ntn_reg_phy))
		{
			ret = PTR_ERR(ntn_reg_phy);

			if (ret == -EPROBE_DEFER)
			{
				NMSGPR_ALERT( "%s: ntn_reg_phy probe deferred!\n", __func__);
			}
			else
			{
				NMSGPR_ALERT( "%s: Get ntn_reg_phy failed!\n", __func__);
			}
			goto exit_error;
		}
		else
		{
			ret = regulator_set_voltage(ntn_reg_phy, NTN_VREG_PHY_MIN, NTN_VREG_PHY_MAX);
			if (ret)
			{
				NMSGPR_ALERT( "%s: Set ntn_reg_phy failed!\n", __func__);
				goto exit_error;
			}
			else
			{
				ret = regulator_enable(ntn_reg_phy);
				if (ret)
				{
					NMSGPR_ALERT( "%s: Enable ntn_reg_phy failed!\n", __func__);
					goto exit_error;
				}
			}
		}
	}
	else
	{
		NMSGPR_ALERT( "%s: power supply %s not found!\n", __func__,NTN_VREG_PHY_NAME);
	}

	return ret;

exit_error:
	if (ntn_reg_phy)
		regulator_put(ntn_reg_phy);
	if (ntn_reg_core)
		regulator_put(ntn_reg_core);
	if (ntn_reg_io)
		regulator_put(ntn_reg_io);
	if (ntn_reg_pci)
		regulator_put(ntn_reg_pci);
	if (ntn_reg_hsic)
		regulator_put(ntn_reg_hsic);
	return ret;
}

/*!
* \brief Initialize and configure GPIOs
*
* \details Parse the device tree for GPIO nodes controlling
* power supplies and Neutrino reset. Missing nodes are non fatal
* because some platforms like IVI don't use GPIOs for power control.
*
* \param[in] idev - struct device from PCI device
*
* \return 0 on success. Negative error code on failure
*/
static int DWC_ETH_QOS_init_gpios(struct device *idev)
{
	int ret = 0;

	if (!idev)
	{
		return -EINVAL;
	}

	/* Configure GPIOs required for GPIO regulator control */
	if (of_find_property(idev->of_node, NTN_PWR_GPIO_NAME, NULL))
	{
		supply_gpio_num = ret = of_get_named_gpio(idev->of_node, NTN_PWR_GPIO_NAME, 0);
		if (ret >= 0)
		{
			ret = gpio_request(supply_gpio_num, NTN_PWR_GPIO_NAME);
			if (ret)
			{
				NMSGPR_ALERT("%s: Can't get GPIO %s, ret = %d\n", __func__, NTN_PWR_GPIO_NAME, supply_gpio_num);
				goto DWC_ETH_QOS_err_init_gpios;
			}

			ret = gpio_direction_output(supply_gpio_num, 0x1);
			if (ret)
			{
				NMSGPR_ALERT("%s: Can't set GPIO %s direction, ret = %d\n", __func__, NTN_PWR_GPIO_NAME, ret);
				goto DWC_ETH_QOS_err_pwr_gpio_request;
			}

			gpio_set_value(supply_gpio_num, 0x1);
		}
		else
		{
			if (ret == -EPROBE_DEFER)
				NMSGPR_ALERT("get NTN_PWR_GPIO_NAME probe defer\n");
			else
				NMSGPR_ALERT("can't get gpio %s ret %d", NTN_PWR_GPIO_NAME, ret);
			goto DWC_ETH_QOS_err_init_gpios;
		}
	}
	else
	{
		NMSGPR_ALERT("can't find gpio %s", NTN_PWR_GPIO_NAME);
	}

	/* Configure GPIOs required for the Neutrino reset */
	if (of_find_property(idev->of_node, NTN_RESET_GPIO_NAME, NULL))
	{
		resx_gpio_num = ret = of_get_named_gpio(idev->of_node, NTN_RESET_GPIO_NAME, 0);
		if (ret >= 0)
		{
			ret = gpio_request(resx_gpio_num, NTN_RESET_GPIO_NAME);
			if (ret)
			{
				NMSGPR_ALERT("%s: Can't get GPIO %s, ret = %d\n", __func__, NTN_RESET_GPIO_NAME, resx_gpio_num);
				goto DWC_ETH_QOS_err_pwr_gpio_request;
			}

			ret = gpio_direction_output(resx_gpio_num, 0x1);
			if (ret)
			{
				NMSGPR_ALERT("%s: Can't set GPIO %s direction, ret = %d\n", __func__, NTN_RESET_GPIO_NAME, ret);
				goto DWC_ETH_QOS_err_resx_gpio_request;
			}

			gpio_set_value(resx_gpio_num, 0x1);
		}
		else
		{
			if (ret == -EPROBE_DEFER)
				NMSGPR_ALERT("get NTN_RESET_GPIO_NAME probe defer\n");
			else
				NMSGPR_ALERT("can't get gpio %s ret %d", NTN_RESET_GPIO_NAME, ret);
			goto DWC_ETH_QOS_err_pwr_gpio_request;
		}
	}
	else
	{
		NMSGPR_ALERT("can't find gpio %s", NTN_RESET_GPIO_NAME);
	}

	return ret;

DWC_ETH_QOS_err_resx_gpio_request:
	gpio_free(resx_gpio_num);
	resx_gpio_num = -1;
DWC_ETH_QOS_err_pwr_gpio_request:
	gpio_free(supply_gpio_num);
	supply_gpio_num = -1;
DWC_ETH_QOS_err_init_gpios:
	return ret;
}

static void DWC_ETH_QOS_free_regs_and_gpios(void)
{
	if (gpio_is_valid(resx_gpio_num)) {
		NMSGPR_ALERT("%s: release gpio = %d\n", __func__, resx_gpio_num);
		gpio_free(resx_gpio_num);
	}

	if (gpio_is_valid(supply_gpio_num)) {
		NMSGPR_ALERT("%s: release gpio = %d\n", __func__, supply_gpio_num);
		/* Power off Neutrino when it is not in use */
		gpio_set_value(supply_gpio_num, 0x0);
		gpio_free(supply_gpio_num);
	}

	if (ntn_reg_phy) {
		NMSGPR_ALERT("%s: drop PHY regulator\n", __func__);
		regulator_put(ntn_reg_phy);
	}

	if (ntn_reg_core) {
		NMSGPR_ALERT("%s: drop CORE regulator\n", __func__);
		regulator_put(ntn_reg_core);
	}

	if (ntn_reg_io) {
		NMSGPR_ALERT("%s: drop IO regulator\n", __func__);
		regulator_put(ntn_reg_io);
	}

	if (ntn_reg_pci) {
		NMSGPR_ALERT("%s: drop PCI regulator\n", __func__);
		regulator_put(ntn_reg_pci);
	}

	if (ntn_reg_hsic) {
		NMSGPR_ALERT("%s: drop HSIC regulator\n", __func__);
		regulator_put(ntn_reg_hsic);
	}
}

static int DWC_ETH_QOS_pcie_probe(struct platform_device *pdev)
{
	int ret = 0;
	unsigned int ntn_rst_delay=100,rc_num=1;
	struct device *idev = &pdev->dev;

	NMSGPR_ALERT("Entry: %s\n", __func__);

	/* Neutrino board init */
	ret = DWC_ETH_QOS_init_regulators(idev);
	if (ret)
	{
		NMSGPR_ALERT("%s: Failed to init regulators\n", __func__);
		goto DWC_ETH_QOS_err_pcie_probe;
	}

	ret = DWC_ETH_QOS_init_gpios(idev);
	if (ret)
	{
		NMSGPR_ALERT("%s: Failed to init GPIOs\n", __func__);
		goto DWC_ETH_QOS_err_pcie_probe;
	}

	/* add delay for neutrino reset propagation*/
	ret = of_property_read_u32(idev->of_node, "qcom,ntn-rst-delay-msec", &ntn_rst_delay);
	if (ret) {
		NMSGPR_ALERT( "%s: Failed to find ntn_rst_delay value, hardcoding to 100 msec\n", __func__);
		ntn_rst_delay = 500;
	}
	else
	{
		NMSGPR_ALERT( "%s: ntn_rst_delay = %d msec\n", __func__,ntn_rst_delay);
	}
	msleep(ntn_rst_delay);

	/* issue PCIe enumeration */
	ret = of_property_read_u32(idev->of_node, "qcom,ntn-rc-num", &rc_num);
	if (ret) {
		NMSGPR_ALERT( "%s: Failed to find PCIe RC number!, RC=%d\n", __func__,rc_num);
		goto DWC_ETH_QOS_err_pcie_probe;
	}
	else {
		NMSGPR_ALERT( "%s: Found PCIe RC number! %d\n", __func__,rc_num);
		ret = msm_pcie_enumerate(rc_num);
		if (ret) {
			NMSGPR_ALERT( "%s: Failed to enable PCIe RC%x!\n", __func__, rc_num);
			goto DWC_ETH_QOS_err_pcie_probe;
		}
		else {
			NMSGPR_ALERT( "%s: PCIe enumeration for RC number %d successful!\n", __func__,rc_num);
		}
	}

	/* register Ethernet modules */
	ret = DWC_ETH_QOS_init_module();
	if (ret) {
		goto DWC_ETH_QOS_err_pcie_probe;
	}

	NMSGPR_ALERT( "Exit:%s success.\n", __func__);
	return ret;

DWC_ETH_QOS_err_pcie_probe:
	DWC_ETH_QOS_free_regs_and_gpios();
	NMSGPR_ALERT( "Exit:%s.\n", __func__);
	return ret;
}

static int DWC_ETH_QOS_pcie_remove(struct platform_device *pdev)
{
	int ret = 0;
	NMSGPR_ALERT( "Entry:%s.\n", __func__);

	DWC_ETH_QOS_exit_module();

	DWC_ETH_QOS_free_regs_and_gpios();

	NMSGPR_ALERT( "Exit:%s.\n", __func__);
	return ret;
}

static struct of_device_id DWC_ETH_QOS_pcie_match[] = {
	{	.compatible = "qcom,ntn_avb",
	},
	{}
};

static struct platform_driver DWC_ETH_QOS_pcie_driver = {
	.probe	= DWC_ETH_QOS_pcie_probe,
	.remove	= DWC_ETH_QOS_pcie_remove,
	.driver	= {
		.name		= "ntn_avb",
		.owner		= THIS_MODULE,
		.of_match_table	= DWC_ETH_QOS_pcie_match,
	},
};

static int __init DWC_ETH_QOS_pcie_init(void)
{
	int ret = 0;

	NMSGPR_ALERT("Entry:%s.\n", __func__);
	ret = platform_driver_register(&DWC_ETH_QOS_pcie_driver);
	NMSGPR_ALERT( "Exit:%s.\n", __func__);
	return ret;
}

static void __exit DWC_ETH_QOS_pcie_exit(void)
{
	NMSGPR_ALERT( "Entry:%s.\n", __func__);
	platform_driver_unregister(&DWC_ETH_QOS_pcie_driver);
	NMSGPR_ALERT( "Exit:%s.\n", __func__);
}

module_init(DWC_ETH_QOS_pcie_init);
module_exit(DWC_ETH_QOS_pcie_exit);
