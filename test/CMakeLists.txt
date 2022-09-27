 
find_package(Qt5Test REQUIRED)

enable_testing()

add_executable(testDomSupport tst_domsupport.cpp)
add_test(NAME testDomSupport COMMAND testDomSupport)
target_link_libraries(testDomSupport PRIVATE libqreadability Qt5::Core Qt5::Test Qt5::Qml)
