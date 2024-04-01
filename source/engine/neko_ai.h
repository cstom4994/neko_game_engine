
#ifndef NEKO_AI_H
#define NEKO_AI_H

#include "engine/neko_engine.h"

/*

//=== Utility AI ===//

    Modeling Curves (examples, all [0, 1]):
        * Binary:   y = x < m ? 0 : 1;
        * Linear:   (100 - x) / 100
        * Cubic:    (100 - x^3) / (100^3)
        * Logistic: 1/(1 + (2.718 * 0.45) ^ (x + 40))
        * Logit:    log_e(x/(1-x) + 5) / 10

    Idea:
        * Collect current states of independent variables
        * NOrmalize using response/modeling curves
        * Combine as necessary
        * Compare all normalized values and select:
            - Highest/lowest selection
            - Weighted random from all choices
            - Weighted random from top/bottom n choices

    Table of utility scores with actions to take based on those scores. These actions could be behavior trees or single tasks.

    Reasoner:
        * Works on a modular list of choices/actions and reports which one to act on
        * Actions are determined by individual 'considerations'
        * Combined considerations generate an 'appraisal'
            - Appraisals are evaluated and the best fit for the time/context is chosen

    Considerations:
        * Considerations are atomic pieces of logic that combined can
            score to help determine which action to take
        * Easy to add/remove considerations from the list at ANY time
        * Easy to extend with new considerations
        * Easy to reuse considerations
        * Encapsulates on aspect of a larger decision
            - Distance
            - Cost
            - Selection History
            - Benefit
            - Health
            - Etc.
        * Parameterized for each individual actor for modularity and granularity of control

//=== Behavior Tree ===//

    typedef enum
    {
        BSF_BT_RUNNING = 0x00,
        BSF_BT_SUCCESS,
        BSF_BT_FAILURE
    } bsf_bt_result;

    Main types of nodes:
        * Composite: One or more children
        * Decorator: Single child
        * Leaf:      No children

    Composites:
        * Sequence: Breadth first walk through children, if all return success then return success. Else return failure.     (Acts as an AND).
        * Selector: Breadth first walk through children, if any return success, return success. Else return failure.         (Acts as an OR).

    Decorators:
        * Inverter:                  Invert result of reported child node state.
        * Repeater:                  Repeat a set number of loops. Typically used at root of tree to continuously loop behavior.
        * Repeat Until Success/Fail: Repeat indefinitely until either success/failure achieved.
        * Succeeder/Failer:          Always return success/failure.
*/

// General context structure used for agents
typedef struct neko_ai_context_t {
    void* user_data;
} neko_ai_context_t;

//==================//
//=== Utility AI ===//

//=== Response Curves ===//
typedef float (*neko_ai_curve_func)(float m, float k, float c, float b, float x);
NEKO_API_DECL float neko_ai_curve_logit(float m, float k, float c, float b, float x);
NEKO_API_DECL float neko_ai_curve_logistic(float m, float k, float c, float b, float x);
NEKO_API_DECL float neko_ai_curve_sin(float m, float k, float c, float b, float x);
NEKO_API_DECL float neko_ai_curve_cos(float m, float k, float c, float b, float x);
NEKO_API_DECL float neko_ai_curve_linearquad(float m, float k, float c, float b, float x);
NEKO_API_DECL float neko_ai_curve_binary(float m, float k, float c, float b, float x);

typedef struct {
    neko_ai_curve_func func;
    float slope;     // Slope of curve
    float exponent;  // Order or curve
    float shift_x;   // Shift curve along x-axis
    float shift_y;   // Shift curve along y-axis
} neko_ai_utility_response_curve_desc_t;

//=== Considerations ===//

typedef struct {
    float data;
    float min;
    float max;
    neko_ai_utility_response_curve_desc_t curve;
} neko_ai_utility_consideration_desc_t;

typedef struct {
    neko_ai_utility_consideration_desc_t* considerations;
    size_t size;
} neko_ai_utility_action_desc_t;

//=== Evaluation ===//
float neko_ai_utility_action_evaluate(neko_ai_utility_action_desc_t* desc);

//=====================//
//=== Behavior Tree ===//

#define NEKO_AI_BT_NODE_MAX_CHILDREN 20
#define NEKO_AI_BT_STATE_FAILURE -1
#define NEKO_AI_BT_STATE_SUCCESS 0
#define NEKO_AI_BT_STATE_RUNNING 1

typedef struct neko_ai_bt_node_t {
    const char* name;
    uint16_t idx;
    int16_t processed_child;
    uint16_t num_children;
    uint16_t max_children;
    int16_t state;
} neko_ai_bt_node_t;

typedef struct neko_ai_bt_t {
    uint16_t current_idx;
    neko_dyn_array(uint32_t) parent_stack;
    neko_dyn_array(neko_ai_bt_node_t) stack;
    neko_ai_context_t ctx;
} neko_ai_bt_t;

typedef void (*neko_ai_bt_leaf_func)(struct neko_ai_bt_t* ctx, struct neko_ai_bt_node_t* node);
typedef bool (*neko_ai_bt_condition_func)(struct neko_ai_bt_t* ctx, struct neko_ai_bt_node_t* node);

NEKO_API_DECL void neko_ai_bt_begin(struct neko_ai_bt_t* ctx);
NEKO_API_DECL void neko_ai_bt_end(struct neko_ai_bt_t* ctx);
NEKO_API_DECL void neko_ai_bt_children_begin(struct neko_ai_bt_t* ctx, struct neko_ai_bt_node_t* parent);
NEKO_API_DECL void neko_ai_bt_children_end(struct neko_ai_bt_t* ctx);
NEKO_API_DECL neko_ai_bt_node_t* neko_ai_bt_parent_node_get(struct neko_ai_bt_t* ctx);

NEKO_API_DECL int16_t neko_ai_bt_repeater_begin(struct neko_ai_bt_t* ctx, uint32_t* count);
NEKO_API_DECL void neko_ai_bt_repeater_end(struct neko_ai_bt_t* ctx);
NEKO_API_DECL int16_t neko_ai_bt_inverter_begin(struct neko_ai_bt_t* ctx);
NEKO_API_DECL void neko_ai_bt_inverter_end(struct neko_ai_bt_t* ctx);
NEKO_API_DECL int16_t neko_ai_bt_condition_begin(struct neko_ai_bt_t* ctx, bool condition);
NEKO_API_DECL void neko_ai_bt_condition_end(struct neko_ai_bt_t* ctx);
NEKO_API_DECL int16_t neko_ai_bt_selector_begin(struct neko_ai_bt_t* ctx);
NEKO_API_DECL void neko_ai_bt_selector_end(struct neko_ai_bt_t* ctx);
NEKO_API_DECL int16_t neko_ai_bt_sequence_begin(struct neko_ai_bt_t* ctx);
NEKO_API_DECL void neko_ai_bt_sequence_end(struct neko_ai_bt_t* ctx);
NEKO_API_DECL int32_t neko_ai_bt_parallel_begin(struct neko_ai_bt_t* ctx);
NEKO_API_DECL void neko_ai_bt_parallel_end(struct neko_ai_bt_t* ctx);
NEKO_API_DECL void neko_ai_bt_leaf(struct neko_ai_bt_t* ctx, neko_ai_bt_leaf_func func);
NEKO_API_DECL void neko_ai_bt_wait(struct neko_ai_bt_t* ctx, float* time, float dt, float max);

NEKO_API_DECL void neko_ai_bt_free(struct neko_ai_bt_t* ctx);

#define neko_ai_bt(_CTX, ...)     \
    do {                          \
        neko_ai_bt_begin((_CTX)); \
        __VA_ARGS__               \
        neko_ai_bt_end((_CTX));   \
    } while (0)

#define neko_ai_repeater(_CTX, ...)                    \
    do {                                               \
        if (neko_ai_bt_repeater_begin((_CTX), NULL)) { \
            __VA_ARGS__                                \
            neko_ai_bt_repeater_end((_CTX));           \
        }                                              \
    } while (0)

#define neko_ai_inverter(_CTX, ...)              \
    do {                                         \
        if (neko_ai_bt_inverter_begin((_CTX))) { \
            __VA_ARGS__                          \
            neko_ai_bt_inverter_end((_CTX));     \
        }                                        \
    } while (0)

#define neko_ai_condition(_CTX, _COND, ...)                \
    do {                                                   \
        if (neko_ai_bt_condition_begin((_CTX), (_COND))) { \
            __VA_ARGS__                                    \
            neko_ai_bt_condition_end((_CTX));              \
        }                                                  \
    } while (0)

#define neko_ai_selector(_CTX, ...)              \
    do {                                         \
        if (neko_ai_bt_selector_begin((_CTX))) { \
            __VA_ARGS__                          \
            neko_ai_bt_selector_end((_CTX));     \
        }                                        \
    } while (0)

#define neko_ai_sequence(_CTX, ...)              \
    do {                                         \
        if (neko_ai_bt_sequence_begin((_CTX))) { \
            __VA_ARGS__                          \
            neko_ai_bt_sequence_end((_CTX));     \
        }                                        \
    } while (0)

#define neko_ai_leaf(_CTX, _FUNC) neko_ai_bt_leaf((_CTX), (_FUNC))
#define neko_ai_wait(_CTX, _TIME, _DT, _MAX) neko_ai_bt_wait((_CTX), (_TIME), (_DT), (_MAX))

#endif  // NEKO_AI_H
