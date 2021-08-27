/*
 * Middlewares.h
 *
 * Created: 8/23/2021 3:39:18 PM
 *  Author: musialny
 */ 

#ifndef MIDDLEWARES_H_
#define MIDDLEWARES_H_

#include "HTTPServer.h"

namespace Middlewares {
	HttpMiddleware* getMiddlewares();
}

#endif /* MIDDLEWARES_H_ */
