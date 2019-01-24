#include "mbvkToolkit.hpp"






int main(int argc,char** argv)
{
	try {

		std::vector<const char*> InstlayerNames = {
			"VK_LAYER_NV_optimus",
			"VK_LAYER_LUNARG_api_dump",
			"VK_LAYER_LUNARG_screenshot",
			"VK_LAYER_RENDERDOC_Capture",
			//"VK_LAYER_VALVE_steam_overlay",
			"VK_LAYER_LUNARG_standard_validation",
		};

		std::vector<const char*> instExtensionNames = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
			//VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		};

		//  check layer & extensions.
		mbvk::Instance_FindLayersProperties(InstlayerNames);
		mbvk::Instance_FindExtensionProperties(instExtensionNames);

		// Instance Vulkan
		mbvk::OBJ_Instance Instance = mbvk::OBJ_Instance("vkTest", instExtensionNames);


		// Physical device found Vulkan
		std::vector<VkPhysicalDevice> vk_physicalDevices;
		mbvk::FindPhysicalDevice(Instance.vk_Instanse, &vk_physicalDevices);
		std::cout << "Vulkan found physical device count:  " << vk_physicalDevices.size() << std::endl;
		
		// Physical device choose
		mbvk::OBJ_PhysicalDevice PhysicalDevice = mbvk::OBJ_PhysicalDevice(vk_physicalDevices[0]);

		// Create device
		mbvk::OBJ_Device Device = mbvk::OBJ_Device(PhysicalDevice, { VK_KHR_SWAPCHAIN_EXTENSION_NAME,VK_EXT_DEBUG_MARKER_EXTENSION_NAME });

		// create window win32api
		mbtk::WindowWin32 window("mbVulkan test", 800, 600);


		// create surface
		mbvk::OBJ_Surface Surface = mbvk::OBJ_Surface(Instance);
		Surface.Create(window.hINSTANCE, window.hWIND);


		while (1) // warning !!! I will soon implement
		{
			window.PoolEvents();
		}

	}
	catch (const char* e) 
	{
		cout << "Error application: " << e << endl;
	}

	

	system("pause");
	return 0;
}