#include "mbvkToolkit.hpp"


using namespace std;


#if defined (_WIN32)
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif




// all toolkit function
namespace mbtk
{
	// mb_programmer: Line equality 
	bool LINE_EQUALITY(const char* s1, const char* s2) {
		if (strcmp(s1, s2) == 0)
			return true;
		return false;
	}


	//-------------------------------------------------------------------------------------------------------------------------

	// WINDOW WIN32 //

#if defined VK_USE_PLATFORM_WIN32_KHR

	// mb_programmer: constructor.
	WindowWin32::WindowWin32()
	{
		hWIND = nullptr;
		hINSTANCE = GetModuleHandleA(nullptr);
		Width = 500;
		Height = 500;
		nameWindow = "mbVulkan";
	}

	// mb_programmer: constructor.
	WindowWin32::WindowWin32(const char* title, int w, int h)
	{
		hWIND = nullptr;
		hINSTANCE = GetModuleHandleA(nullptr);
		Width = 500;
		Height = 500;
		nameWindow = title;

		Setup();

	}

	// mb_programmer: destructor.
	WindowWin32::~WindowWin32()
	{

	}

	// mb_programmer: massage window.
	void WindowWin32::PoolEvents()
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{

			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
	}

	// mb_programmer: settings.
	void WindowWin32::Setup()
	{
		LPCSTR windowClassName = "window class";
		
		WNDCLASSA windowClassInfo;
		{
			windowClassInfo.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			windowClassInfo.lpfnWndProc = DefWindowProcA;
			windowClassInfo.cbClsExtra = 0;
			windowClassInfo.cbWndExtra = 0;
			windowClassInfo.hInstance = hINSTANCE;
			windowClassInfo.hIcon = nullptr;
			windowClassInfo.hCursor = nullptr;
			windowClassInfo.hbrBackground = nullptr;
			windowClassInfo.lpszMenuName = nullptr;
			windowClassInfo.lpszClassName = windowClassName;
		}

		if (!RegisterClassA(&windowClassInfo))
		{
			throw std::exception("failed to register window class");
		}
		

		hWIND = CreateWindowA(
			windowClassName,
			nameWindow,
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			0, 0, Width, Height,
			nullptr,
			nullptr,
			hINSTANCE,
			nullptr
			);

		if (!hWIND)
		{
			throw std::exception("failed to create window");
		}

		
	}


#endif


	//-------------------------------------------------------------------------------------------------------------------------


	// WINDOW XCB //

#if defined VK_USE_PLATFORM_XCB_KHR

	// mb_programmer: constructor.
	WindowXCB::WindowXCB()
	{
#if defined _WIN32
		MessageBox(NULL, L"This not system Linux!!! Choose class WindowWin32 ",L"Warning",MB_OK|MB_ICONINFORMATION);
#endif
	}

	// mb_programmer: destructor.
	WindowXCB::~WindowXCB()
	{
	}


	// mb_programmer: settings window.
	void WindowXCB::Setup()
	{

		uint32_t value_mask, value_list[32];

		window = xcb_generate_id(connection);

		value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

		value_list[0] = screen->black_pixel;
		value_list[1] =
			XCB_EVENT_MASK_KEY_RELEASE |
			XCB_EVENT_MASK_KEY_PRESS |
			XCB_EVENT_MASK_EXPOSURE |
			XCB_EVENT_MASK_STRUCTURE_NOTIFY |
			XCB_EVENT_MASK_POINTER_MOTION |
			XCB_EVENT_MASK_BUTTON_PRESS |
			XCB_EVENT_MASK_BUTTON_RELEASE;

		if (settings.fullscreen)
		{
			Width = destWidth = screen->width_in_pixels;
			Height = destHeight = screen->height_in_pixels;
		}

		xcb_create_window(connection,
			XCB_COPY_FROM_PARENT,
			window, screen->root,
			0, 0, Width, Height, 0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			screen->root_visual,
			value_mask, value_list);

		/* Magic code that will send notification when window is destroyed */
		xcb_intern_atom_reply_t* reply = intern_atom_helper(connection, true, "WM_PROTOCOLS");
		atom_wm_delete_window = intern_atom_helper(connection, false, "WM_DELETE_WINDOW");

		xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
			window, (*reply).atom, 4, 32, 1,
			&(*atom_wm_delete_window).atom);

		std::string windowTitle = getWindowTitle();
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
			window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
			title.size(), windowTitle.c_str());

		free(reply);

		if (settings.fullscreen)
		{
			xcb_intern_atom_reply_t *atom_wm_state = intern_atom_helper(connection, false, "_NET_WM_STATE");
			xcb_intern_atom_reply_t *atom_wm_fullscreen = intern_atom_helper(connection, false, "_NET_WM_STATE_FULLSCREEN");
			xcb_change_property(connection,
				XCB_PROP_MODE_REPLACE,
				window, atom_wm_state->atom,
				XCB_ATOM_ATOM, 32, 1,
				&(atom_wm_fullscreen->atom));
			free(atom_wm_fullscreen);
			free(atom_wm_state);
		}

		xcb_map_window(connection, window);


	}

#endif




}// end namespace mbtk...


// vulkan toolkit
namespace mbvk
{


	// mb_programmer: Find layers properties Vulkan.
	std::vector<VkLayerProperties> Instance_FindLayersProperties(std::vector<const char*>& findLaerProp)
	{
		VkResult result = VK_SUCCESS;
		uint32_t layer_count = 0;
		std::vector<VkLayerProperties> FoundLaerProp;

		// count layers.
		result = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		if (result != VK_SUCCESS) throw std::exception("Vulkan could not get layers instance! ");

		// query all layers.
		FoundLaerProp.resize(layer_count);
		result = vkEnumerateInstanceLayerProperties(&layer_count, FoundLaerProp.data());
		if (result != VK_SUCCESS) throw std::exception("Vulkan could not write layers instance! ");

		// check layers supporting.
		for (const char* &nameFindLayer : findLaerProp)
		{
			bool check = false;
			for (const VkLayerProperties& nameFoundLayer : FoundLaerProp)
			{
				if (mbtk::LINE_EQUALITY(nameFindLayer, nameFoundLayer.layerName)) {
					check = true;
					break;
				}
			}
			
			if (!check)
				cout << "Vilkan instance not supporting layer name: " << nameFindLayer << endl;

		}



		return FoundLaerProp;
	}

	// mb_programmer: Get extension properties Vulkan.
	std::vector<VkExtensionProperties> Instance_FindExtensionProperties(std::vector<const char*>& findExtensionProp)
	{
		VkResult result = VK_SUCCESS;
		uint32_t extension_count = 0;
		std::vector<VkExtensionProperties> foundExtensionProp;


		// count extension.
		result = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
		if (result != VK_SUCCESS) throw std::exception("Vulkan could not get extension instance! ");

		// save data extension.
		foundExtensionProp.resize(extension_count);
		result = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, foundExtensionProp.data());
		if (result != VK_SUCCESS) throw std::exception("Vulkan could not write extension instance! ");


		// check extension supporting.
		for (const char* &nameFindExtension : findExtensionProp)
		{
			bool check = false;
			for (const VkExtensionProperties& nameFoundExtension : foundExtensionProp)
			{
				if (mbtk::LINE_EQUALITY(nameFindExtension, nameFoundExtension.extensionName)) {
					check = true;
					break;
				}
			}

			if (!check)
				cout << "Vilkan instance not supporting extension name: " << nameFindExtension << endl;

		}



		return foundExtensionProp;
	}



	// mb_programming: Find physic device in sysytem.
	uint32_t FindPhysicalDevice(const VkInstance& instansVulkan, std::vector<VkPhysicalDevice> *dataPhysicalDevice)
	{
		// 0 initialize variables.
		VkResult result = VK_SUCCESS;
		uint32_t physical_devices_count = 0;

		// 1 call.
		result = vkEnumeratePhysicalDevices(instansVulkan, &physical_devices_count, nullptr);
		if (result != VK_SUCCESS) 
			throw std::exception("Vulkan not found physic device!");
		

		// 2 call.
		if (dataPhysicalDevice != nullptr) 
		{
			dataPhysicalDevice->resize(physical_devices_count);
			result = vkEnumeratePhysicalDevices(instansVulkan, &physical_devices_count, dataPhysicalDevice->data());
			if (result != VK_SUCCESS) 
				throw std::exception("Vulkan not success write found physic device!");
		}
		
		
		return physical_devices_count;
	}

	// mb_programmer: Let's see what extensions this physical device supports.
	VkResult PhysicalDevice_FindExtensionProperties(const VkPhysicalDevice& physicalDevice, std::vector<VkExtensionProperties>& dataExtensionProp)
	{
		// 0 initialize variables.
		VkResult result = VK_SUCCESS;
		uint32_t device_extension_prop_count = 0;

		// 1 call.
		result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &device_extension_prop_count, nullptr);
		if (result != VK_SUCCESS) return result;

		// 2 call.
		dataExtensionProp.resize(device_extension_prop_count);
		result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &device_extension_prop_count, dataExtensionProp.data());
		if (result != VK_SUCCESS) return result;




		return result;
	}


	// mb_programmer: See which family queues are supported by this selected physical. device.
	uint32_t PhysicaDevice_FindQueueFamilyProperties(const VkPhysicalDevice& physicalDevice, std::vector<VkQueueFamilyProperties>& dataQueueFamilyprop)
	{
		uint32_t physical_device_queue_family_properties_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &physical_device_queue_family_properties_count, nullptr);

		dataQueueFamilyprop.resize(physical_device_queue_family_properties_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &physical_device_queue_family_properties_count, dataQueueFamilyprop.data());

		return physical_device_queue_family_properties_count;
	}


	// mb_programmer: Get the queue family index that supports the requested queue flags.
	uint32_t PhysicaDevice_FindQueueFamilyIndex(VkQueueFlagBits queueFlags, const std::vector<VkQueueFamilyProperties>& dataQueueFamilyprop)
	{
		// Выделенная очередь для вычисления
		// Попробуйте найти индекс семейства очереди, который поддерживает вычисление, но не графику
		if (queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(dataQueueFamilyprop.size()); i++)
			{
				if ((dataQueueFamilyprop[i].queueFlags & queueFlags) && ((dataQueueFamilyprop[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					return i;
					break;
				}
			}
		}

		// Выделенная очередь для вычисления
		// Попробуйте найти индекс семейства очереди, который поддерживает вычисление, но не графику
		if (queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(dataQueueFamilyprop.size()); i++)
			{
				if ((dataQueueFamilyprop[i].queueFlags & queueFlags) && ((dataQueueFamilyprop[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					return i;
					break;
				}
			}
		}

		// Для других типов очередей или если нет отдельной очереди вычислений, верните первый, чтобы поддерживать запрошенные флаги
		for (uint32_t i = 0; i < static_cast<uint32_t>(dataQueueFamilyprop.size()); i++)
		{
			if (dataQueueFamilyprop[i].queueFlags & queueFlags)
			{
				return i;
				break;
			}
		}

		throw std::runtime_error("Could not find a matching queue family index");
	}

	// mb_programmer: fined surface format.
	uint32_t SurfaceKHR_FindFormat(VkSurfaceKHR Surface, const VkPhysicalDevice& physicalDevice, std::vector<VkSurfaceFormatKHR>& dataFormat)
	{

		uint32_t FormatsCount;
		

		if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, Surface, &FormatsCount, nullptr) != VkResult::VK_SUCCESS)
		{
			throw std::exception("Vulkan failed to get surface formats count");
		}

		dataFormat.resize(FormatsCount);
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, Surface, &FormatsCount, dataFormat.data()) != VkResult::VK_SUCCESS)
		{
			throw std::exception("Vulkan failed to get surface formats");
		}

		return FormatsCount;
	}


	// mb_programmer: find swapchain image.
	uint32_t SwapchainKHR_FindImage(const VkDevice& Devise, const VkSwapchainKHR& Swapchain, std::vector<VkImage>& dataImage)
	{
		uint32_t ImagesCount;
		if (vkGetSwapchainImagesKHR(Devise, Swapchain, &ImagesCount, nullptr) != VkResult::VK_SUCCESS)
		{
			throw std::exception("failed to get swapchain images count");
		}

		dataImage.resize(ImagesCount);
		if (vkGetSwapchainImagesKHR(Devise, Swapchain, &ImagesCount, dataImage.data()) != VkResult::VK_SUCCESS)
		{
			throw std::exception("failed to get swapchain images");
		}

		return ImagesCount;
	}







	//-----------------------------------------------------------------------------------------------------------------------------------------




	// 1. VULKAN INSTANCE //

	// mb_programmer: constructor.
	OBJ_Instance::OBJ_Instance()
	{
		vk_Instanse = VK_NULL_HANDLE;
		
	}

	// mb_programmer: constructor.
	OBJ_Instance::OBJ_Instance(const char* AppName, std::vector<const char*>& dataExtension, std::vector<const char*> *dataLayer )
	{
		vk_Instanse = VK_NULL_HANDLE;
	
		// create instance vulkan
		Setup(AppName, dataExtension, dataLayer);
		
	}

	// mb_programmer: destructor.
	mbvk::OBJ_Instance::~OBJ_Instance()
	{
		// warning: least delete!
		vkDestroyInstance(vk_Instanse, nullptr);
	}
	

	// mb_programmer: cretae instance vulkan.
	void mbvk::OBJ_Instance::Setup(const char* AppName, std::vector<const char*>& dataExtension, std::vector<const char*> *dataLayer)
	{
		VkApplicationInfo vk_applicationInfo;
		{
			vk_applicationInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO;
			vk_applicationInfo.pNext = nullptr;
			vk_applicationInfo.pApplicationName = AppName;
			vk_applicationInfo.applicationVersion = 1;
			vk_applicationInfo.pEngineName = "mbvkEngine";
			vk_applicationInfo.engineVersion = 1;
			vk_applicationInfo.apiVersion = VK_VERSION_1_0;
		}
		VkInstanceCreateInfo vk_instanceCreateInfo;
		{
			vk_instanceCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			vk_instanceCreateInfo.pNext = nullptr;
			vk_instanceCreateInfo.flags = 0;
			vk_instanceCreateInfo.pApplicationInfo = &vk_applicationInfo;
			if (dataLayer != nullptr) 
			{
				vk_instanceCreateInfo.enabledLayerCount = dataLayer->size(); // fill
				vk_instanceCreateInfo.ppEnabledLayerNames = dataLayer->data(); // fill
			}
			else 
			{
				vk_instanceCreateInfo.enabledLayerCount = 0;
				vk_instanceCreateInfo.ppEnabledLayerNames = nullptr;
			}
			vk_instanceCreateInfo.enabledExtensionCount = dataExtension.size();
			vk_instanceCreateInfo.ppEnabledExtensionNames = dataExtension.data();
		}

		// cretae instance vulkan.
		if (vkCreateInstance(&vk_instanceCreateInfo, nullptr, &vk_Instanse) == VkResult::VK_SUCCESS)
		{
			cout << "Vulkan create instance!" << endl;
			
		}
		else
			throw std::exception("failed to create vk instance");
		
		
	}


	//-----------------------------------------------------------------------------------------------------------------------------------------


	// 2 VULKAN PHYSIC DEVICE //

	// mb_programmer: constructor.
	OBJ_PhysicalDevice::OBJ_PhysicalDevice()
	{
		vk_PhysicalDevice = VK_NULL_HANDLE;
	}
	// mb_programmer: constructor.
	OBJ_PhysicalDevice::OBJ_PhysicalDevice(VkPhysicalDevice& physicalDevice)
	{
		vk_PhysicalDevice = VK_NULL_HANDLE;
		vk_PhysicalDevice = physicalDevice;
		Setup();
	}
	// mb_programmer: destructor.
	OBJ_PhysicalDevice::~OBJ_PhysicalDevice()
	{

	}

	// mb_programmer: settings.
	void OBJ_PhysicalDevice::Setup()
	{
		// Let's see what extensions this physical device supports.
		PhysicalDevice_FindExtensionProperties(vk_PhysicalDevice, vk_Extension);


		// Properties and memory properties of a physical device (so that examples can test them)
		vkGetPhysicalDeviceProperties(vk_PhysicalDevice, &vk_Properties);				// returns device properties
		vkGetPhysicalDeviceFeatures(vk_PhysicalDevice, &vk_Features);					// returns supports the functionality.
		vkGetPhysicalDeviceMemoryProperties(vk_PhysicalDevice, &vk_MemoryProperties);	// returns device memory properties.

		// Find queue family properties.
		mbvk::PhysicaDevice_FindQueueFamilyProperties(vk_PhysicalDevice, vk_QueueFamilyProperties);

#if defined _DEBUG
		cout << endl;
		cout << "QueryFamily count: " << vk_QueueFamilyProperties.size() << endl;
		cout << "QueryFamily 0 queueCount: " << vk_QueueFamilyProperties[0].queueCount << endl;
		cout << "QueryFamily 1 queueCount: " << vk_QueueFamilyProperties[1].queueCount << endl;
#endif
		

	}



	//-----------------------------------------------------------------------------------------------------------------------------------------


	// 3 VULKAN DEVICE //

	// mb_programmer: constructor.
	OBJ_Device::OBJ_Device()
	{
		vk_Device = VK_NULL_HANDLE;
		vk_CommandPool = VK_NULL_HANDLE;
	}

	// mb_programmer: constructor.
	OBJ_Device::OBJ_Device(const OBJ_PhysicalDevice& physicalDevice, std::vector<const char*> deviceExtensions)
	{
		vk_Device = VK_NULL_HANDLE;
		vk_CommandPool = VK_NULL_HANDLE;

		// found graphics queue.
		vk_QueueFamilyIndices.graphics = PhysicaDevice_FindQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, physicalDevice.vk_QueueFamilyProperties);
		vk_QueueFamilyIndices.compute = PhysicaDevice_FindQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, physicalDevice.vk_QueueFamilyProperties);
		vk_QueueFamilyIndices.transfer = PhysicaDevice_FindQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, physicalDevice.vk_QueueFamilyProperties);

#if defined _DEBUG
		cout << endl;
		cout << "QueueFamilyIndices VK_QUEUE_GRAPHICS_BIT: " << vk_QueueFamilyIndices.graphics << endl;
		cout << "QueueFamilyIndices VK_QUEUE_COMPUTE_BIT: " << vk_QueueFamilyIndices.compute << endl;
		cout << "QueueFamilyIndices VK_QUEUE_TRANSFER_BIT: " << vk_QueueFamilyIndices.transfer << endl;
#endif

		// settings devise.
		Setup();

#if defined _DEBUG
		cout << endl;
		for (const VkExtensionProperties &res : physicalDevice.vk_Extension) {
			cout << "All device supporting extension name: " << res.extensionName << endl;
		}
#endif

		// check requested extensions device.		
		for (const char* &currentCheckExtension : deviceExtensions)
		{
			bool check_not_extension = false;
			for (const VkExtensionProperties &res : physicalDevice.vk_Extension) 
				if (mbtk::LINE_EQUALITY(currentCheckExtension, res.extensionName)) {
					check_not_extension = true;
					extensions_found.push_back(currentCheckExtension);
					break;
				}
			

			if(!check_not_extension)
				cout << "Device not supporting extension name => " << currentCheckExtension << endl;

		}
			

		// create logic device.
		Create(physicalDevice);
		

	}

	// mb_programmer: destructor.
	OBJ_Device::~OBJ_Device()
	{
		vkDestroyDevice(vk_Device, nullptr);
	}

	// mb_programmer: settings device queue.
	void OBJ_Device::Setup()
	{
		
		// queue priority.
		const float defaultQueuePriority(0.0f);
		
		// Graphic queue
		VkDeviceQueueCreateInfo queueGraphicsInfo{};
		queueGraphicsInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueGraphicsInfo.queueFamilyIndex = vk_QueueFamilyIndices.graphics;
		queueGraphicsInfo.queueCount = 1;
		queueGraphicsInfo.pQueuePriorities = &defaultQueuePriority;
		vk_queueCreateInfos.push_back(queueGraphicsInfo);


		// Calculation queue.
		if (vk_QueueFamilyIndices.compute != vk_QueueFamilyIndices.graphics)
		{
			
			VkDeviceQueueCreateInfo queueComputesInfo{};
			queueComputesInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueComputesInfo.queueFamilyIndex = vk_QueueFamilyIndices.compute;
			queueComputesInfo.queueCount = 1;
			queueComputesInfo.pQueuePriorities = &defaultQueuePriority;
			vk_queueCreateInfos.push_back(queueComputesInfo);
		}
		else {
			// In addition, we use the same queue.
			vk_QueueFamilyIndices.compute = vk_QueueFamilyIndices.graphics;
		}


		// Transfer queue
		if ( (vk_QueueFamilyIndices.transfer != vk_QueueFamilyIndices.graphics) && (vk_QueueFamilyIndices.transfer != vk_QueueFamilyIndices.compute))
		{
			
			VkDeviceQueueCreateInfo queueTransferInfo{};
			queueTransferInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueTransferInfo.queueFamilyIndex = vk_QueueFamilyIndices.transfer;
			queueTransferInfo.queueCount = 1;
			queueTransferInfo.pQueuePriorities = &defaultQueuePriority;
			vk_queueCreateInfos.push_back(queueTransferInfo);

		}
		else {
			// In addition, we use the same queue.
			vk_QueueFamilyIndices.transfer = vk_QueueFamilyIndices.graphics;
		}





	}

	// mb_programmer: crate  device.
	void OBJ_Device::Create(const OBJ_PhysicalDevice& physicalDevice)
	{
		// Create a logical device based on the designated physical device.
		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(vk_queueCreateInfos.size());;
		deviceCreateInfo.pQueueCreateInfos = vk_queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &physicalDevice.vk_Features;

		if (extensions_found.size() > 0)
		{		
			deviceCreateInfo.enabledExtensionCount = (uint32_t)extensions_found.size();
			deviceCreateInfo.ppEnabledExtensionNames = extensions_found.data();
		}

		VkResult result = vkCreateDevice(physicalDevice.vk_PhysicalDevice, &deviceCreateInfo, nullptr, &vk_Device);

		if (result == VK_SUCCESS)
		{
			// Create default command pool for graphic command buffers
			VkCommandPoolCreateInfo cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = vk_QueueFamilyIndices.graphics;
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			
			if (vkCreateCommandPool(vk_Device, &cmdPoolInfo, nullptr, &vk_CommandPool) != VK_SUCCESS)
				throw std::exception("Vulkan not create command pool device for queue!");
			

			cout << "Vulkan create logic device!" << endl;
		}
	}


	//-----------------------------------------------------------------------------------------------------------------------------------------

	
	// 4 VULKAN SURFACE //

	// mb_programmer: constructor.
	OBJ_Surface::OBJ_Surface()
	{
		vk_Surface = VK_NULL_HANDLE;
		vk_Instance = VK_NULL_HANDLE;
	}

	OBJ_Surface::OBJ_Surface(const OBJ_Instance &Istance)
	{
		vk_Surface = VK_NULL_HANDLE;
		vk_Instance = Istance.vk_Instanse;

	}

	// mb_programmer: destructor.
	OBJ_Surface::~OBJ_Surface()
	{
		vkDestroySurfaceKHR(vk_Instance, vk_Surface, nullptr);
	}
	
	// mb_programmer: constructor for windowsWin32 .
	void OBJ_Surface::Create(mbtk::WindowWin32 &pWindow)
	{
		// Windows system.
#if defined VK_USE_PLATFORM_WIN32_KHR

		VkResult error = VK_SUCCESS;

		VkWin32SurfaceCreateInfoKHR vk_win32SurfaceCreateInfoKHR = {};
		vk_win32SurfaceCreateInfoKHR.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		vk_win32SurfaceCreateInfoKHR.pNext = nullptr;
		vk_win32SurfaceCreateInfoKHR.flags = 0;
		vk_win32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)pWindow.hINSTANCE; //GetModuleHandle(NULL);
		vk_win32SurfaceCreateInfoKHR.hwnd = (HWND)pWindow.hWIND;
		
		if (vkCreateWin32SurfaceKHR(vk_Instance, &vk_win32SurfaceCreateInfoKHR, nullptr, &vk_Surface) != VkResult::VK_SUCCESS)
		{
			throw std::exception("Vulkan failed to create surfaceWin32");
		}
	
		cout << "Vulkan create surfaceWin32!" << endl;
#endif

	}

	// mb_programmer: Linux system. 
#if defined(VK_USE_PLATFORM_XCB_KHR)
	// mb_programmer: constructor for windowsXCB.
	void OBJ_Surface::Create(mbtk::WindowXCB &pWindow)
	{
		
		VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.connection = pWindow.connection;
		surfaceCreateInfo.window = pWindow.window;
		if (vkCreateXcbSurfaceKHR(vk_Instance, &surfaceCreateInfo, nullptr, &vk_Surface) != VkResult::VK_SUCCESS)
		{
			throw std::exception("Vulkan could not create surfaceXCB!");
		}

		cout << "Vulkan create surface XCB!" << endl;

	}
#endif

	void OBJ_Surface::Format(const OBJ_PhysicalDevice& physicalDevice)
	{
		// 1
		uint32_t count = SurfaceKHR_FindFormat(vk_Surface, physicalDevice.vk_PhysicalDevice, vk_Formats);
		
	
		cout << "Vilkan Surface format count: " << count << endl;


		// 2
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.vk_PhysicalDevice, vk_Surface, &vk_Capabilities) != VkResult::VK_SUCCESS)
		{
			throw std::exception("Vulkan failed to get surface capabilities");
		}

	}


	//-----------------------------------------------------------------------------------------------------------------------------------------


	// 5 VULKAN SWAPCHAIN //

	// mb_programmer: constructor.
	OBJ_Swapchain::OBJ_Swapchain()
	{
		vk_Swapchain = VK_NULL_HANDLE;
	}

	// mb_programmer: destructor.
	OBJ_Swapchain::~OBJ_Swapchain()
	{
		//vkDestroySwapchainKHR()
	}

	// mb_programming: settings.
	void OBJ_Swapchain::Create(const OBJ_Surface &Surface, const OBJ_Device& Device)
	{

		VkSwapchainCreateInfoKHR vk_SwapchainCreateInfoKHR;
		{
			
			vk_SwapchainCreateInfoKHR.sType = VkStructureType::VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			vk_SwapchainCreateInfoKHR.pNext = nullptr;
			vk_SwapchainCreateInfoKHR.flags = 0;
			vk_SwapchainCreateInfoKHR.surface = Surface.vk_Surface;
			vk_SwapchainCreateInfoKHR.minImageCount = 2;
			vk_SwapchainCreateInfoKHR.imageFormat = Surface.vk_Formats[0].format;
			vk_SwapchainCreateInfoKHR.imageColorSpace = Surface.vk_Formats[0].colorSpace;
			vk_SwapchainCreateInfoKHR.imageExtent = VkExtent2D{
				Surface.vk_Capabilities.currentExtent.width,
				Surface.vk_Capabilities.currentExtent.height
			};
			vk_SwapchainCreateInfoKHR.imageArrayLayers = 1;
			vk_SwapchainCreateInfoKHR.imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			vk_SwapchainCreateInfoKHR.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
			vk_SwapchainCreateInfoKHR.queueFamilyIndexCount = 0;
			vk_SwapchainCreateInfoKHR.pQueueFamilyIndices = nullptr;
			vk_SwapchainCreateInfoKHR.preTransform = VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			vk_SwapchainCreateInfoKHR.compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			vk_SwapchainCreateInfoKHR.presentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
			vk_SwapchainCreateInfoKHR.clipped = VK_TRUE;
			vk_SwapchainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;
		}

		if (vkCreateSwapchainKHR(Device.vk_Device, &vk_SwapchainCreateInfoKHR, nullptr, &vk_Swapchain) != VkResult::VK_SUCCESS)
		{
			throw std::exception("Vulkan failed to create swapchain");
		}

		cout << "Vulkan Seapchan!" << endl;

		// image swapchain.
		SwapchainKHR_FindImage(Device.vk_Device, vk_Swapchain, vk_SwapchainImages);

		cout << "Vulkan Seapchan image count: " << vk_SwapchainImages .size() << endl;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------


}// end namespace mbvk...



#if defined (_WIN32)

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE: // Handle window resizing
	{
		int width = LOWORD(lParam);  // Macro to get the low-order word.
		int height = HIWORD(lParam); // Macro to get the high-order word.

									 //std::cout << width << std::endl;
									 // Respond to the message:
									 //OnSize(hwnd, (UINT)wParam, width, height);
	}
	break;
	case WM_CLOSE:
	{
		if (MessageBox(NULL, L"Really quit?", L"My application", MB_OKCANCEL) > 0)
		{
			DestroyWindow(hwnd);
		}
	}
	break;


	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);;

}

#endif