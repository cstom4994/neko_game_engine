
#ifndef NEKO_AI_H
#define NEKO_AI_H

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

#define gsai_bt(_CTX, ...)      \
    do {                        \
        neko_ai_bt_begin((_CTX)); \
        __VA_ARGS__             \
        neko_ai_bt_end((_CTX));   \
    } while (0)

#define gsai_repeater(_CTX, ...)                     \
    do {                                             \
        if (neko_ai_bt_repeater_begin((_CTX), NULL)) { \
            __VA_ARGS__                              \
            neko_ai_bt_repeater_end((_CTX));           \
        }                                            \
    } while (0)

#define gsai_inverter(_CTX, ...)               \
    do {                                       \
        if (neko_ai_bt_inverter_begin((_CTX))) { \
            __VA_ARGS__                        \
            neko_ai_bt_inverter_end((_CTX));     \
        }                                      \
    } while (0)

#define gsai_condition(_CTX, _COND, ...)                 \
    do {                                                 \
        if (neko_ai_bt_condition_begin((_CTX), (_COND))) { \
            __VA_ARGS__                                  \
            neko_ai_bt_condition_end((_CTX));              \
        }                                                \
    } while (0)

#define gsai_selector(_CTX, ...)               \
    do {                                       \
        if (neko_ai_bt_selector_begin((_CTX))) { \
            __VA_ARGS__                        \
            neko_ai_bt_selector_end((_CTX));     \
        }                                      \
    } while (0)

#define gsai_sequence(_CTX, ...)               \
    do {                                       \
        if (neko_ai_bt_sequence_begin((_CTX))) { \
            __VA_ARGS__                        \
            neko_ai_bt_sequence_end((_CTX));     \
        }                                      \
    } while (0)

#define gsai_leaf(_CTX, _FUNC) neko_ai_bt_leaf((_CTX), (_FUNC))
#define gsai_wait(_CTX, _TIME, _DT, _MAX) neko_ai_bt_wait((_CTX), (_TIME), (_DT), (_MAX))

/** @} */  // end of neko_ai_util

//========================//
/*==== Implementation ====*/

#ifdef NEKO_AI_IMPL

//=====================//
//=== Behavior Tree ===//

NEKO_API_DECL void neko_ai_bt_free(struct neko_ai_bt_t* ctx) {
    if (ctx->parent_stack) neko_dyn_array_free(ctx->parent_stack);
    if (ctx->stack) neko_dyn_array_free(ctx->stack);
}

NEKO_API_DECL void neko_ai_bt_begin(struct neko_ai_bt_t* ctx) { ctx->current_idx = 0; }

NEKO_API_DECL void neko_ai_bt_end(struct neko_ai_bt_t* ctx) { neko_assert(neko_dyn_array_empty(ctx->parent_stack)); }

NEKO_API_DECL neko_ai_bt_node_t* neko_ai_bt_parent_node_get(struct neko_ai_bt_t* ctx) {
    if (neko_dyn_array_empty(ctx->parent_stack)) return NULL;

    uint32_t idx = ctx->parent_stack[neko_dyn_array_size(ctx->parent_stack) - 1];
    return &ctx->stack[idx];
}

NEKO_API_DECL void neko_ai_bt_children_begin(struct neko_ai_bt_t* ctx, struct neko_ai_bt_node_t* parent) {
    parent->num_children = 0;
    neko_dyn_array_push(ctx->parent_stack, parent->idx);
    ctx->current_idx = parent->idx + 1;
}

NEKO_API_DECL void neko_ai_bt_children_end(struct neko_ai_bt_t* ctx) { neko_dyn_array_pop(ctx->parent_stack); }

NEKO_API_DECL neko_ai_bt_node_t* neko_ai_bt_node_child_get(struct neko_ai_bt_t* ctx, struct neko_ai_bt_node_t* parent, uint32_t child_index) {
    neko_ai_bt_node_t* node = NULL;
    uint32_t ci = parent->idx + child_index + 1;
    uint32_t sz = neko_dyn_array_size(ctx->stack);
    if (ci < sz) {
        node = &ctx->stack[ci];
    }
    return node;
}

NEKO_API_DECL neko_ai_bt_node_t* neko_ai_bt_node_next(struct neko_ai_bt_t* ctx) {
    // Push on new node if doesn't exist
    neko_ai_bt_node_t* np = NULL;

    uint32_t cnt = neko_dyn_array_size(ctx->stack);
    if (ctx->current_idx >= neko_dyn_array_size(ctx->stack)) {
        neko_ai_bt_node_t node = neko_default_val();
        node.state = NEKO_AI_BT_STATE_RUNNING;
        node.idx = ctx->current_idx++;
        node.num_children = 0;
        node.processed_child = 0;
        neko_dyn_array_push(ctx->stack, node);
        np = &ctx->stack[neko_dyn_array_size(ctx->stack) - 1];
    } else {
        // Get pointer to node and increment idx
        np = &ctx->stack[ctx->current_idx];
        np->idx = ctx->current_idx++;
    }

    // Increment children of current parent, if available
    neko_ai_bt_node_t* parent = neko_ai_bt_parent_node_get(ctx);
    if (parent) {
        // Increase number of children
        parent->num_children++;

        // If we're concerned about max children, make sure we haven't passed it
        if (parent->max_children && parent->num_children > parent->max_children) {
            neko_log_error("Attempting to add more children than max allows.");
        }
    }

    return np;
}

NEKO_API_DECL int32_t neko_ai_bt_parallel_begin(struct neko_ai_bt_t* ctx) {
    // Get next node in stack
    neko_ai_bt_node_t* node = neko_ai_bt_node_next(ctx);
    node->name = "parallel";
    neko_ai_bt_node_t* parent = neko_ai_bt_parent_node_get(ctx);

    // If not processing this node, return 0x00 to fail if check
    if (parent && parent->processed_child != -1 && neko_ai_bt_node_child_get(ctx, parent, parent->processed_child)->idx != node->idx) return 0x00;

    if (node->state == NEKO_AI_BT_STATE_RUNNING) {
        // Begin processing new child stack
        neko_ai_bt_children_begin(ctx, node);
    }

    // Processed child for parallel will be -1, to signify that we process ALL children.
    node->processed_child = -1;

    return node->state;
}

NEKO_API_DECL void neko_ai_bt_parallel_end(struct neko_ai_bt_t* ctx) {
    // Get top of parent stack for node
    neko_ai_bt_node_t* node = neko_ai_bt_parent_node_get(ctx);

    // For each child
    bool all_succeed = true;
    for (uint32_t i = 0; i < node->num_children; ++i) {
        neko_ai_bt_node_t* child = neko_ai_bt_node_child_get(ctx, node, i);
        neko_assert(child);

        // If any child fails, this node fails
        if (child->state == NEKO_AI_BT_STATE_RUNNING) {
            all_succeed = false;
        }
        if (child->state == NEKO_AI_BT_STATE_FAILURE) {
            node->state = NEKO_AI_BT_STATE_FAILURE;
            all_succeed = false;
            break;
        }
    }

    // If we've failed, we don't need to process anymore children
    if (node->state == NEKO_AI_BT_STATE_FAILURE || !node->num_children) {
        node->state = NEKO_AI_BT_STATE_FAILURE;

        // Pop all children and their children off stack
        uint32_t cnt = neko_dyn_array_size(ctx->stack);
        for (uint32_t i = node->idx + 1; i < cnt; ++i) {
            neko_dyn_array_pop(ctx->stack);
        }
        ctx->current_idx = node->idx + 1;
    } else if (!all_succeed) {
        node->state = NEKO_AI_BT_STATE_RUNNING;
    } else {
        node->state = NEKO_AI_BT_STATE_SUCCESS;

        // Pop all children and their children off stack
        uint32_t cnt = neko_dyn_array_size(ctx->stack);
        for (uint32_t i = node->idx + 1; i < cnt; ++i) {
            neko_dyn_array_pop(ctx->stack);
        }
        ctx->current_idx = node->idx + 1;
    }

    // End child stack, pop off transient parent stack
    neko_ai_bt_children_end(ctx);
}

NEKO_API_DECL int16_t neko_ai_bt_selector_begin(struct neko_ai_bt_t* ctx) {
    // Get next node in stack
    neko_ai_bt_node_t* node = neko_ai_bt_node_next(ctx);
    node->name = "selector";
    neko_ai_bt_node_t* parent = neko_ai_bt_parent_node_get(ctx);

    // If not processing this node, return 0x00 to fail if check
    if (parent && parent->processed_child != -1 && neko_ai_bt_node_child_get(ctx, parent, parent->processed_child)->idx != node->idx) return 0x00;

    if (node->state == NEKO_AI_BT_STATE_RUNNING) {
        // Begin processing new child stack
        neko_ai_bt_children_begin(ctx, node);
    }

    return node->state;
}

NEKO_API_DECL void neko_ai_bt_selector_end(struct neko_ai_bt_t* ctx) {
    // Get top of parent stack for node
    neko_ai_bt_node_t* node = neko_ai_bt_parent_node_get(ctx);

    for (uint32_t i = 0; i < node->num_children; ++i) {
        node->processed_child = i;
        neko_ai_bt_node_t* child = neko_ai_bt_node_child_get(ctx, node, i);
        neko_assert(child);

        if (child->state == NEKO_AI_BT_STATE_RUNNING) {
            break;
        } else if (child->state == NEKO_AI_BT_STATE_SUCCESS) {
            node->state = NEKO_AI_BT_STATE_SUCCESS;
            break;
        } else if (child->state == NEKO_AI_BT_STATE_FAILURE && i == node->num_children - 1) {
            node->processed_child = node->num_children;
            break;
        }
    }

    // If we're successful, we don't need to process anymore children
    if (node->state == NEKO_AI_BT_STATE_SUCCESS || !node->num_children) {
        node->state = NEKO_AI_BT_STATE_SUCCESS;

        // Pop all children off stack
        for (uint32_t i = 0; i < node->num_children; ++i) {
            neko_dyn_array_pop(ctx->stack);
        }
        ctx->current_idx = node->idx + 1;
    }
    // If processed_child < num_children, then we're still processing/running
    else if ((int32_t)node->processed_child < (int32_t)node->num_children) {
        node->state = NEKO_AI_BT_STATE_RUNNING;
    } else {
        node->state = NEKO_AI_BT_STATE_FAILURE;

        // Pop off all children off stack
        for (uint32_t i = 0; i < node->num_children; ++i) {
            neko_dyn_array_pop(ctx->stack);
        }
        ctx->current_idx = node->idx + 1;
    }

    // End child stack, pop off transient parent stack
    neko_ai_bt_children_end(ctx);
}

NEKO_API_DECL int16_t neko_ai_bt_sequence_begin(struct neko_ai_bt_t* ctx) {
    // Get next node in stack
    neko_ai_bt_node_t* node = neko_ai_bt_node_next(ctx);
    neko_ai_bt_node_t* parent = neko_ai_bt_parent_node_get(ctx);
    node->name = "sequence";

    // If not processing this node, return 0x00 to fail if check
    if (parent && parent->processed_child != -1 && neko_ai_bt_node_child_get(ctx, parent, parent->processed_child)->idx != node->idx) return 0x00;

    if (node->state == NEKO_AI_BT_STATE_RUNNING) {
        // Begin processing new child stack
        neko_ai_bt_children_begin(ctx, node);
    }

    return node->state;
}

NEKO_API_DECL void neko_ai_bt_sequence_end(struct neko_ai_bt_t* ctx) {
    // Get top of parent stack for node
    neko_ai_bt_node_t* node = neko_ai_bt_parent_node_get(ctx);

    // For each child
    for (uint32_t i = 0; i < node->num_children; ++i) {
        node->processed_child = i;
        neko_ai_bt_node_t* child = neko_ai_bt_node_child_get(ctx, node, i);
        neko_assert(child);

        if (child->state == NEKO_AI_BT_STATE_RUNNING) {
            break;
        } else if (child->state == NEKO_AI_BT_STATE_FAILURE) {
            node->state = NEKO_AI_BT_STATE_FAILURE;
            break;
        } else if (child->state == NEKO_AI_BT_STATE_SUCCESS && i == node->num_children - 1) {
            node->processed_child = node->num_children;
            break;
        }
    }

    // If we've failed, we don't need to process anymore children
    if (node->state == NEKO_AI_BT_STATE_FAILURE || !node->num_children) {
        node->state = NEKO_AI_BT_STATE_FAILURE;

        // Pop all children off stack, reset current idx?
        for (uint32_t i = 0; i < node->num_children; ++i) {
            neko_dyn_array_pop(ctx->stack);
        }
        ctx->current_idx = node->idx + 1;
    }
    // If processed_child < num_children, then we're still processing/running
    else if ((int32_t)node->processed_child < (int32_t)node->num_children) {
        node->state = NEKO_AI_BT_STATE_RUNNING;
    } else {
        node->state = NEKO_AI_BT_STATE_SUCCESS;

        // Pop off all children off stack
        for (uint32_t i = 0; i < node->num_children; ++i) {
            neko_dyn_array_pop(ctx->stack);
        }
        ctx->current_idx = node->idx + 1;
    }

    // End child stack, pop off transient parent stack
    neko_ai_bt_children_end(ctx);
}

NEKO_API_DECL int16_t neko_ai_bt_repeater_begin(struct neko_ai_bt_t* ctx, uint32_t* count) {
    // Get next node in stack
    neko_ai_bt_node_t* node = neko_ai_bt_node_next(ctx);
    node->name = "repeater";
    neko_ai_bt_node_t* parent = neko_ai_bt_parent_node_get(ctx);

    // If not processing this node, return 0x00 to fail if check
    if (parent && neko_ai_bt_node_child_get(ctx, parent, parent->processed_child)->idx != node->idx) return 0x00;

    // Set max children
    node->max_children = 1;

    if (node->state != NEKO_AI_BT_STATE_RUNNING) {
        if (count && *count) {
            (*count)--;
            if (*count) node->state = NEKO_AI_BT_STATE_RUNNING;
        } else if (!count) {
            node->state = NEKO_AI_BT_STATE_RUNNING;
        }
    }

    if (node->state == NEKO_AI_BT_STATE_RUNNING) {
        // Begin processing new child stack
        neko_ai_bt_children_begin(ctx, node);
    }

    return node->state;
}

NEKO_API_DECL void neko_ai_bt_repeater_end(struct neko_ai_bt_t* ctx) {
    // Get top of parent stack for node
    neko_ai_bt_node_t* node = neko_ai_bt_parent_node_get(ctx);

    for (uint32_t i = 0; i < node->num_children; ++i) {
        node->processed_child = i;
        neko_ai_bt_node_t* child = neko_ai_bt_node_child_get(ctx, node, i);
        neko_assert(child);
        node->state = child->state;
    }

    if (node->state != NEKO_AI_BT_STATE_RUNNING) {
        node->state = NEKO_AI_BT_STATE_SUCCESS;

        // Pop all children and their children off stack
        uint32_t cnt = neko_dyn_array_size(ctx->stack);
        for (uint32_t i = node->idx + 1; i < cnt; ++i) {
            neko_dyn_array_pop(ctx->stack);
        }
        ctx->current_idx = node->idx + 1;
    }

    // End child stack, pop off transient parent stack
    neko_ai_bt_children_end(ctx);
}

NEKO_API_DECL int16_t neko_ai_bt_inverter_begin(struct neko_ai_bt_t* ctx) {
    // Get next node in stack
    neko_ai_bt_node_t* node = neko_ai_bt_node_next(ctx);
    node->name = "inverter";
    neko_ai_bt_node_t* parent = neko_ai_bt_parent_node_get(ctx);

    // If not processing this node, return 0x00 to fail if check
    if (parent && neko_ai_bt_node_child_get(ctx, parent, parent->processed_child)->idx != node->idx) return 0x00;

    // Set max children
    node->max_children = 1;

    if (node->state == NEKO_AI_BT_STATE_RUNNING) {
        // Begin processing new child stack
        neko_ai_bt_children_begin(ctx, node);
    }

    return node->state;
}

NEKO_API_DECL void neko_ai_bt_inverter_end(struct neko_ai_bt_t* ctx) {
    // Get top of parent stack for node
    neko_ai_bt_node_t* node = neko_ai_bt_parent_node_get(ctx);

    for (uint32_t i = 0; i < node->num_children; ++i) {
        node->processed_child = i;
        neko_ai_bt_node_t* child = neko_ai_bt_node_child_get(ctx, node, i);
        neko_assert(child);
        switch (child->state) {
            case NEKO_AI_BT_STATE_RUNNING:
                node->state = NEKO_AI_BT_STATE_RUNNING;
                break;
            case NEKO_AI_BT_STATE_FAILURE:
                node->state = NEKO_AI_BT_STATE_SUCCESS;
                break;
            case NEKO_AI_BT_STATE_SUCCESS:
                node->state = NEKO_AI_BT_STATE_FAILURE;
                break;
        }
    }

    if (node->state != NEKO_AI_BT_STATE_RUNNING) {
        // Pop all children and their children off stack
        uint32_t cnt = neko_dyn_array_size(ctx->stack);
        for (uint32_t i = node->idx + 1; i < cnt; ++i) {
            neko_dyn_array_pop(ctx->stack);
        }
        ctx->current_idx = node->idx + 1;
    }

    // End child stack, pop off transient parent stack
    neko_ai_bt_children_end(ctx);
}

NEKO_API_DECL int16_t neko_ai_bt_condition_begin(struct neko_ai_bt_t* ctx, bool condition) {
    // Get next node in stack
    neko_ai_bt_node_t* node = neko_ai_bt_node_next(ctx);
    node->name = "condition";
    neko_ai_bt_node_t* parent = neko_ai_bt_parent_node_get(ctx);

    // If not processing this node, return 0x00 to fail if check
    if (parent && neko_ai_bt_node_child_get(ctx, parent, parent->processed_child)->idx != node->idx) return 0x00;

    // Set max children
    node->max_children = 1;

    if (node->state == NEKO_AI_BT_STATE_RUNNING) {
        if (condition) {
            // Begin processing new child stack
            neko_ai_bt_children_begin(ctx, node);
        } else {
            node->state = NEKO_AI_BT_STATE_FAILURE;
        }
    }

    return (node->state != NEKO_AI_BT_STATE_FAILURE);
}

NEKO_API_DECL void neko_ai_bt_condition_end(struct neko_ai_bt_t* ctx) {
    // Get top of parent stack for node
    neko_ai_bt_node_t* node = neko_ai_bt_parent_node_get(ctx);

    for (uint32_t i = 0; i < node->num_children; ++i) {
        node->processed_child = i;
        neko_ai_bt_node_t* child = neko_ai_bt_node_child_get(ctx, node, i);
        neko_assert(child);
        node->state = child->state;  // pass through child state
    }

    if (node->state != NEKO_AI_BT_STATE_RUNNING) {
        // Pop all children and their children off stack
        uint32_t cnt = neko_dyn_array_size(ctx->stack);
        for (uint32_t i = node->idx + 1; i < cnt; ++i) {
            neko_dyn_array_pop(ctx->stack);
        }
        ctx->current_idx = node->idx + 1;
    }

    // End child stack, pop off transient parent stack
    neko_ai_bt_children_end(ctx);
}

NEKO_API_DECL void neko_ai_bt_leaf(struct neko_ai_bt_t* ctx, neko_ai_bt_leaf_func func) {
    // Next node
    neko_ai_bt_node_t* node = neko_ai_bt_node_next(ctx);
    neko_ai_bt_node_t* parent = neko_ai_bt_parent_node_get(ctx);
    node->name = "leaf";

    // If not processing this node, return 0x00 to fail if check
    if (parent && parent->processed_child != -1 && neko_ai_bt_node_child_get(ctx, parent, parent->processed_child)->idx != node->idx) return;

    func(ctx, node);
}

NEKO_API_DECL void neko_ai_bt_wait(struct neko_ai_bt_t* ctx, float* time, float dt, float max) {
    // Next node
    neko_ai_bt_node_t* node = neko_ai_bt_node_next(ctx);
    neko_ai_bt_node_t* parent = neko_ai_bt_parent_node_get(ctx);
    node->name = "wait";

    // If not processing this node, return 0x00 to fail if check
    if (parent && parent->processed_child != -1 && neko_ai_bt_node_child_get(ctx, parent, parent->processed_child)->idx != node->idx) return;

    if (!time) {
        node->state = NEKO_AI_BT_STATE_SUCCESS;
    } else if (max == 0.f) {
        node->state = NEKO_AI_BT_STATE_SUCCESS;
    } else {
        node->state = NEKO_AI_BT_STATE_RUNNING;

        *time += dt;
        if (*time >= max) {
            node->state = NEKO_AI_BT_STATE_SUCCESS;
        }
    }
}

//==================//
//=== Utility AI ===//

//=== Response Curves ===//

NEKO_API_DECL float neko_ai_curve_logit(float m, float k, float c, float b, float x) {
    if (k == 0.f) k = 0.0001f;
    float z = (x / k) - c;
    return 0.5f * (log(z / (1.f - z)) / log(pow(100.f, m))) + b + 0.5f;
}

NEKO_API_DECL float neko_ai_curve_logistic(float m, float k, float c, float b, float x) {
    float z = 10 * m * (x - c - 0.5f);
    return k * (1.f / (1.f + pow(2.7183f, -z))) + b;
}

NEKO_API_DECL float neko_ai_curve_sin(float m, float k, float c, float b, float x) { return m * sin(pow((x - c), k)) + b; }

NEKO_API_DECL float neko_ai_curve_cos(float m, float k, float c, float b, float x) { return m * cos(pow((x - c), k)) + b; }

NEKO_API_DECL float neko_ai_curve_linearquad(float m, float k, float c, float b, float x) { return m * pow((x - c), k) + b; }

NEKO_API_DECL float neko_ai_curve_binary(float m, float k, float c, float b, float x) { return x < m ? k : c; }

float neko_ai_utility_action_evaluate(neko_ai_utility_action_desc_t* desc) {
    float val = 1.f;
    uint32_t cnt = desc->size / sizeof(neko_ai_utility_consideration_desc_t);
    for (uint32_t i = 0; i < cnt; ++i) {
        neko_ai_utility_consideration_desc_t* cp = &desc->considerations[i];

        // Normalize range from bookends to [0.f, 1.f]
        float v = neko_map_range(cp->min, cp->max, 0.f, 1.f, cp->data);

        // Run response curve to calculate value
        neko_ai_utility_response_curve_desc_t* c = &cp->curve;
        v = c->func(c->slope, c->exponent, c->shift_x, c->shift_y, v);

        // Calculate compensation factor
        float mod_factor = 1.f - (1.f / cnt);
        float makeup = (1.f - v) * mod_factor;
        v = v + (makeup * v);

        // Final score multiplied to val
        val *= v;
    }
    return val;
}

#undef NEKO_AI_IMPL
#endif  // NEKO_AI_IMPL

#endif  // NEKO_AI_H
