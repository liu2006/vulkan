module;
#include <vector>
module app:validationLayers;

const std::vector<const char *> validationLayers{"VK_LAYER_KHRONOS_validation"};

constexpr bool enableValidationLayers
{
#ifdef NDEBUG
    false
#else
    true
#endif
};

