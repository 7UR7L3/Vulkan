#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <functional>

#include <vector>



/* NOT MY WORK - SEE vulkan-tutorial.com FOR CODE EXPLANATION */

template <typename T>
class VDeleter {
public:
    VDeleter() : VDeleter([](T, VkAllocationCallbacks*) {}) {}

    VDeleter(std::function<void(T, VkAllocationCallbacks*)> deletef) {
        this->deleter = [=](T obj) { deletef(obj, nullptr); };
    }

    VDeleter(const VDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef) {
        this->deleter = [&instance, deletef](T obj) { deletef(instance, obj, nullptr); };
    }

    VDeleter(const VDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef) {
        this->deleter = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
    }

    ~VDeleter() {
        cleanup();
    }

    T* operator &() {
        cleanup();
        return &object;
    }

    operator T() const {
        return object;
    }

private:
    T object{VK_NULL_HANDLE};
    std::function<void(T)> deleter;

    void cleanup() {
        if (object != VK_NULL_HANDLE) {
            deleter(object);
        }
        object = VK_NULL_HANDLE;
    }
};



//	cause finding a good place for these takes effort
const int WIDTH = 666 * 2;
const int HEIGHT = 666;

const std::vector< const char* > validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

#ifndef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class HelloTriangleApplication
{
public:

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
	(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData
	)
	{
		std::cerr << "validation layer: " << msg << std::endl;

		return VK_FALSE;
	}

    void run()
	{
		initWindow();
        initVulkan();
        mainLoop();
    }

private:

	GLFWwindow* window;

	VDeleter< VkInstance > instance{ vkDestroyInstance };
	VkDebugReportCallbackEXT callback;

	void initWindow()
	{
		glfwInit();	//	yes i would like a window please
		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );	//	plz no gl
		glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );	//	cuz resizing is hard lel

		window = glfwCreateWindow( WIDTH, HEIGHT, "Vulkan window name probably", nullptr, nullptr );
	}

	void createInstance()	//	an instance is how the application uses vulkan; "starts" vulkan / connects to vulkan library
	{
		if (enableValidationLayers && !checkValidationLayerSupport()) throw std::runtime_error( "unavailable validation layers requested" );



		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "ima be a triangle when i grow up";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "lul no engine.. i am the engine.. yey me";
		appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );	//	lol?
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else createInfo.enabledLayerCount = 0;


		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) throw std::runtime_error( "failed to create instance" );


		/*
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );
		std::vector< VkExtensionProperties > extensions( extensionCount );
		vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, extensions.data() );

		std::cout << "available extensions :" << std::endl;
		for (const auto& extension : extensions) std::cout << "\t" << extension.extensionName << std::endl;
		*/
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();
	}

	bool checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector< VkLayerProperties > availableLayers( layerCount );
		vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data() );
		
		for (const char* layerName : validationLayers)	//	check that we have everything we need
		{
			bool layerFound = false;
			for( const auto& layerProperties : availableLayers )
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			if (!layerFound) return false;
		}

		return true;
	}

	std::vector< const char* > getRequiredExtensions()
	{
		std::vector< const char* > extensions;

		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );
		
		for (unsigned int i = 0; i < glfwExtensionCount; i++) extensions.push_back( glfwExtensions[i] );

		if (enableValidationLayers) extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );

		return extensions;
	}

	void setupDebugCallback()
	{
		if (!enableValidationLayers) return;

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;
	}

    void initVulkan()
	{
		createInstance();
		setupDebugCallback();
	}

    void mainLoop()
	{
		for (; !glfwWindowShouldClose(window); )
		{
			glfwPollEvents();
		}
    }
};

int main()
{
    HelloTriangleApplication app;

    try
	{
        app.run();
    }
	catch( const std::runtime_error& e )
	{
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}