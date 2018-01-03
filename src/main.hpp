#include <unistd.h>
#include <stdio.h>
#include <cmath>

#include "util/s_string.h"
#include "util/s_printf.h"
#include "util/s_regex.h"
#include "util/s_load.h"
#include "util/s_save.h"
#include "util/assume.h"
#include "util/now.h"
#include "util/Struct.hpp"

#include "util/numeric.hpp"
#include "util/random.hpp"
#include "util/solver.hpp"
#include "util/Histogram.hpp"
#include "util/CurveFit.hpp"
#include "util/ParameterOptimizer.hpp"

#include "network/Input.hpp"
#include "network/BufferedInput.hpp"
#include "network/Transform.hpp"
#include "network/RecurrentTransform.hpp"
#include "network/KernelTransform.hpp"
#include "network/LinearTransform.hpp"
#include "network/LinearNonLinearTransform.hpp"
#include "network/SparseNonLinearTransform.hpp"
#include "network/IntegrateAndFireTransform.hpp"
#include "network/SoftMaxTransform.hpp"

#include "network/TransformCriterion.hpp"
#include "network/SupervisedCriterion.hpp"
#include "network/ObservableCriterion.hpp"
#include "network/KernelEstimator.hpp"
#include "network/KernelExperimentalEstimator.hpp"
#include "network/KernelDistributedEstimator.hpp"
