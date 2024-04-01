
#pragma once

#include <iostream>
#include <map>
#include <type_traits>
#include <vector>

#include "engine/neko.h"
#include "engine/neko_script.h"

neko_script_vector(neko_script_binary_t *) modules = NULL;

#define print_type_name(T) std::cout << __LINE__ << " " << #T << ": " << typeid(T).name() << std::endl;

ns_bind_func_local(test_func, haha) {
    if (argc != 2) neko_script_throw("fuck fuck");

    ns_args(double, num);
    ns_args(const char *, str);

    std::cout << num << " " << str << std::endl;

    ns_ret(100);
}

int neko_vm_interpreter(const_str code_file) {
    neko_script_ctx_t *ctx = neko_script_ctx_new(NULL);
    neko_script_builtin_install(ctx);

    // neko_script_ctx_addvar(ctx, strdup("wssb"), neko_script_value_number(1.444));
    double ff = neko_pi;
    double &r_ff = ff;

    std::vector<float> vec = {55.1, 66.4, 77.1};
    // std::vector vec = {neko_script_value_number(55), neko_script_value_number(66), neko_script_value_number(77)};
    // std::vector<int> vec{};

    std::map<const char *, double> map = {{"one", 11}, {"two", 22}};
    // std::map<const char *, neko_script_value_t *> map = {{"one", neko_script_value_number(11)}, {"two", neko_script_value_number(22)}};

    neko_script_auto_add(ctx, "test_number", r_ff);
    neko_script_auto_add(ctx, "test_str", r_ff);
    neko_script_auto_add(ctx, "test_vector", vec);
    neko_script_auto_add(ctx, "test_map", map);

    struct A {
        double ff;
    };

    A a = {114.514};

    neko_script_auto_add(ctx, "test_native", (ns_userdata_t)&a);

    ns_bind_func_lambda(test_lambda) {
        if (argc != 1) neko_script_throw("fuck fuck");

        neko_script_value_t *str = neko_script_vector_pop(ctx->stack);

        std::cout << str->string << std::endl;

        neko_script_vector_push(ctx->stack, neko_script_auto(100));
        //
        return;
    };

    ns_bind_func_lambda(test_native_data) {
        if (argc != 1) neko_script_throw("fuck fuck");
        ns_args(ns_userdata_t, data);

        A *ra = (A *)data;

        std::cout << ra->ff << std::endl;

        ns_ret(0);
    };

    neko_script_ctx_addfn(ctx, NULL, strdup("test_func"), 1, 0, neko_binding_test_lambda);
    neko_script_ctx_addfn(ctx, NULL, strdup("test_addd"), 2, 0, neko_binding_test_func);
    neko_script_ctx_addfn(ctx, NULL, strdup("test_native_data"), 1, 0, neko_binding_test_native_data);

    // while (1) {
    //     getstring();
    //     neko_script_dis_str(ctx, input_buffer, load_module, NULL);
    //     if (!neko_script_eval_str(ctx, input_buffer, load_module, NULL)) {
    //         // puts(ex_msg);
    //     }
    // }

    neko_script_binary_t *bin;

    // neko_script_dis_str(ctx, code_buffer, load_module, NULL);
    if (NULL == (bin = neko_script_eval_file(ctx, code_file, ns_load_module, NULL, false))) {
        puts(neko_script_ex_msg);
    }

    printf("---------------\n");

    // auto test_ha = neko_script_auto<double>(ctx, "b");
    // auto test_haha = neko_script_auto<char *>(ctx, "c");
    // auto test_hahaha = neko_script_auto<std::vector<double>>(ctx, "test_vector");
    // auto test_hahahaha = neko_script_auto<std::map<const char *, double>>(ctx, "test_map");
    // std::cout << test_ha << std::endl;
    // std::cout << test_haha << std::endl;
    // for (auto &s : test_hahaha) std::cout << s << std::endl;
    // for (auto &[k, v] : test_hahahaha) std::cout << k << " " << v << std::endl;

    neko_script_print_stack(ctx);

    // for (int i = 0; i < 5; i++) {
    //     auto t = neko_script_auto_call<double>(ctx, "test_foo1", 202044, i);
    //     std::cout << t << std::endl;
    // }

    // if (!neko_script_eval_str(ctx, "test_foo2(1234);", load_module, NULL)) {
    //     puts(neko_script_ex_msg);
    // }

    //    neko_script_binary_free(bin);

    neko_script_gc_freeall();
    for (int i = 0; i < neko_script_vector_size(modules); i++) {
        neko_script_binary_free(modules[i]);
    }
    neko_script_vector_free(modules);

    return 0;
}