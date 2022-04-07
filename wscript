# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('lorawan-5g-integration', ['core','internet'])
    module.source = [
        'model/aaa-server.cc',
	'model/access-mobility-management-function.cc',
	'model/application-server.cc',
	'model/authentication-server-function.cc',
	'model/basic-net-device.cc',
	'model/g-gateway.cc',
	'model/basic-struct.cc',
	'model/session-management-function.cc',
	'model/unified-data-management.cc',
	'model/user-plane-function.cc',
    'model/my-one-shot-sender.cc',
	'helper/my-one-shot-sender-helper.cc',
	'helper/basic-net-device-helper.cc'
        ]

    module_test = bld.create_ns3_module_test_library('lorawan-5g-integration')
    module_test.source = [
        'test/lorawan-5g-integration-test-suite.cc',
        ]
    # Tests encapsulating example programs should be listed here
    if (bld.env['ENABLE_EXAMPLES']):
        module_test.source.extend([
        #    'test/lorawan-5g-integration-examples-test-suite.cc',
             ])

    headers = bld(features='ns3header')
    headers.module = 'lorawan-5g-integration'
    headers.source = [
        'model/aaa-server.h',
	'model/access-mobility-management-function.h',
	'model/application-server.h',
	'model/authentication-server-function.h',
	'model/basic-net-device.h',
	'model/g-gateway.h',
	'model/basic-struct.h',
	'model/session-management-function.h',
	'model/unified-data-management.h',
	'model/user-plane-function.h',
	'model/my-one-shot-sender.h',
	'helper/my-one-shot-sender-helper.h',
	'helper/basic-net-device-helper.h'
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

