#ifndef MB_VK_TOOLKIT_HPP
#define MB_VK_TOOLKIT_HPP
/*
* date: 22/01/2019 
* web: mbprogrammer.com
* git: github.com/maximprogram
* developer: Maksim Bogdanoff 
*/

#if defined _MSC_VER && defined _WIN32
	#pragma comment(lib, "vulkan-1.lib")
#endif

#if defined _WIN32
	#include <Windows.h>
	#define VK_USE_PLATFORM_WIN32_KHR 1
#endif


#if defined __linux__
	#include <xcb/xcb.h>
	#define VK_USE_PLATFORM_XCB_KHR 1
#endif

#include <map>
#include <vector>
#include <iostream>

#include "vulkan/vulkan.h"

using std::cout;
using std::endl;
using std::cerr;



#define mbSHIFT_BIT_RIGHT(arg,count_step)( arg >> count_step )
#define mbSHIFT_BIT_LEFT(arg,count_step)( arg << count_step )



namespace mbtk
{
	// mb_programmer: Line equality 
	bool LINE_EQUALITY(const char* s1, const char* s2);

	// mb_programmer: Find element in blocks memory.
	template<typename T>
	bool FIND_ELEMENT(std::vector<T> data, T extension)
	{
		/*
		An iterator of the first element in the range that is compared to val.
		If no items match, the function returns the last one.
		*/
		return (std::find(data.begin(), data.end(), extension) != data.end());
	}



#if defined VK_USE_PLATFORM_WIN32_KHR

	class WindowWin32
	{
	public:
		WindowWin32();
		WindowWin32(const char* title, int w,int h);
		virtual ~WindowWin32();

		void PoolEvents();


		HWND hWIND;
		HINSTANCE hINSTANCE;
		int Width, Height;

	private:
		const char* nameWindow;
		MSG msg;
		void Setup();
	};


#endif


#if defined VK_USE_PLATFORM_XCB_KHR  // <== WARNING: in work!!!

	class WindowXCB
	{
	public:
		WindowXCB();
		virtual ~WindowXCB();



		int Width, Height;
		bool quit = false;
		xcb_connection_t *connection;
		xcb_screen_t *screen;
		xcb_window_t window;
		xcb_intern_atom_reply_t *atom_wm_delete_window;


	private:
		void Setup();
	};

#endif

}




namespace mbvk 
{



	// mb_programmer: Find layers properties Vulkan.
	std::vector<VkLayerProperties> Instance_FindLayersProperties(std::vector<const char*>& dataLaerProp);

	// mb_programmer: Get extension properties Vulkan.
	std::vector<VkExtensionProperties> Instance_FindExtensionProperties(std::vector<const char*>& dataExtensionProp);


	// mb_programming: Find physic device in sysytem.
	uint32_t FindPhysicalDevice(const VkInstance& instansVulkan, std::vector<VkPhysicalDevice> *dataPhysicalDevice = nullptr);

	// mb_programmer: Let's see what extensions this physical device supports.
	VkResult PhysicalDevice_FindExtensionProperties(const VkPhysicalDevice& physicalDevice, std::vector<VkExtensionProperties>& dataExtensionProp);


	// mb_programmer: View which family queues are supported by this selected physical device.
	uint32_t PhysicaDevice_FindQueueFamilyProperties(const VkPhysicalDevice& physicalDevice, std::vector<VkQueueFamilyProperties>& dataQueueFamilyprop);

	// mb_programmer: Get the queue family index that supports the requested queue flags.
	uint32_t PhysicaDevice_FindQueueFamilyIndex(VkQueueFlagBits queueFlags, const std::vector<VkQueueFamilyProperties>& dataQueueFamilyprop);

	// mb_programmer: fined surface format, return function count format.
	uint32_t SurfaceKHR_FindFormat(VkSurfaceKHR vk_Surface, const VkPhysicalDevice& physicalDevice, std::vector<VkSurfaceFormatKHR>& dataFormat);

	// mb_programmer: find swapchain image.
	uint32_t SwapchainKHR_FindImage(const VkDevice& Devise,const VkSwapchainKHR& Swapchain, std::vector<VkImage>& dataImage);




	// 1 VULKAN INSTANCE //
	class OBJ_Instance
	{
	public:
		OBJ_Instance();
		OBJ_Instance(const char* AppName, std::vector<const char*>& dataExtension, std::vector<const char*> *dataLayer = nullptr);
		virtual ~OBJ_Instance();
		
		VkInstance vk_Instanse;
	private:
		
		void Setup(const char* AppName, std::vector<const char*>& dataExtension, std::vector<const char*> *dataLayer = nullptr);
		
	};



	// 2 VULKAN PHYSICAL DEVICE //
	class OBJ_PhysicalDevice
	{

	public:
		OBJ_PhysicalDevice();
		OBJ_PhysicalDevice(VkPhysicalDevice& physicalDevice);
		virtual ~OBJ_PhysicalDevice();

		
		// pointer object physical device.
		VkPhysicalDevice vk_PhysicalDevice;
		// Saves available on selected physical device (Characteristics).
		VkPhysicalDeviceFeatures vk_Features;
		// Preserves the physical properties of the device.
		VkPhysicalDeviceProperties vk_Properties;
		// Saves all available memory (type) properties for physical. devices.
		VkPhysicalDeviceMemoryProperties vk_MemoryProperties;
		// Saves all extensions supported by this physical. device.
		std::vector<VkExtensionProperties> vk_Extension;
		// Stores the number of available queue families.
		std::vector<VkQueueFamilyProperties> vk_QueueFamilyProperties;

				
	private:
		void Setup();

	};



	struct obj_QueueFamilyIndices
	{
		uint32_t graphics;
		uint32_t compute;
		uint32_t transfer;
	};
	
	// 3 VULKAN DEVICE //
	class OBJ_Device
	{
	public:
		OBJ_Device();
		OBJ_Device( const OBJ_PhysicalDevice& physicalDevice, std::vector<const char*> deviceExtensions);
		virtual ~OBJ_Device();

		VkDevice vk_Device;
		// default command pool for graphic queue family index.
		VkCommandPool vk_CommandPool;
		// Queue family indices.
		obj_QueueFamilyIndices vk_QueueFamilyIndices;
		//  Required queues need to be requested when creating logical devices
		std::vector<VkDeviceQueueCreateInfo> vk_queueCreateInfos{};

	private:
		std::vector<const char*> extensions_found{};
		void Setup();
		void Create(const OBJ_PhysicalDevice& physicalDevice);

	};


	// 4 VULKAN SURFACE //
	class OBJ_Surface
	{
	public:
		OBJ_Surface();
		OBJ_Surface(const OBJ_Instance &Istance);
		virtual ~OBJ_Surface();
		
		void Create(mbtk::WindowWin32 &pWindow);

#if defined VK_USE_PLATFORM_XCB_KHR // Linux system. 
		void Create(mbtk::WindowXCB &pWindow);
#endif
		void Format(const OBJ_PhysicalDevice& physicalDevice);
		
		VkSurfaceKHR vk_Surface;
		
		std::vector<VkSurfaceFormatKHR> vk_Formats;
		VkSurfaceCapabilitiesKHR vk_Capabilities;
	private:
		VkInstance vk_Instance;
		
	};


	// 5 VULKAN SWAPCHAIN //
	class OBJ_Swapchain
	{
	public:

		OBJ_Swapchain();
		virtual ~OBJ_Swapchain();
		void Create(const OBJ_Surface &Surface, const OBJ_Device& Device);

		VkSwapchainKHR vk_Swapchain;
		std::vector<VkImage> vk_SwapchainImages;
	private:

		
	};


}


#endif//!MB_VK_TOOLKIT_HPP
