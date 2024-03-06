#ifndef COMMONAPI_FDBUS_TYPES_HPP_
#define COMMONAPI_FDBUS_TYPES_HPP_

#include <fdbus/fdbus.h>
#include <fdbus/IFdbMsgBuilder.h>
using namespace ipc::fdbus;

// typedef int DBusHandlerResult;
typedef void* DBusPendingCall;

#define DBUS_ERROR_UNKNOWN_METHOD             "org.freedesktop.DBus.Error.UnknownMethod"
#define DBUS_ERROR_TIMEOUT                    "org.freedesktop.DBus.Error.Timeout"
#define DBUS_ERROR_DISCONNECTED               "org.freedesktop.DBus.Error.Disconnected"
#define DBUS_TIMEOUT_INFINITE ((int) 0x7fffffff)

typedef enum
{
  DBUS_HANDLER_RESULT_HANDLED,         /**< Message has had its effect - no need to run more handlers. */ 
  DBUS_HANDLER_RESULT_NOT_YET_HANDLED, /**< Message has not had any effect - see if other handlers want it. */
  DBUS_HANDLER_RESULT_NEED_MEMORY      /**< Need more memory in order to return #DBUS_HANDLER_RESULT_HANDLED or #DBUS_HANDLER_RESULT_NOT_YET_HANDLED. Please try again later with more memory. */
} DBusHandlerResult;

#ifndef TRUE
#  define TRUE 1
#endif
#ifndef FALSE
#  define FALSE 0
#endif



#endif

