#pragma once

#define MUTILITY_STRINGIFY(X) MUTILITY_PERFORM_STRINGIFY(X)
#define MUTILITY_PERFORM_STRINGIFY(X) #X

#define MUTILITY_EMPTY_EXPRESSION ((void)0)