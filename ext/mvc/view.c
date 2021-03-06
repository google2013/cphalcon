/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  +------------------------------------------------------------------------+
*/

#include "mvc/view.h"
#include "mvc/viewinterface.h"
#include "mvc/view/engine.h"
#include "mvc/view/engineinterface.h"
#include "mvc/view/engine/php.h"
#include "mvc/view/exception.h"
#include "mvc/view/modelinterface.h"
#include "cache/backendinterface.h"
#include "di/injectable.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/fcall.h"
#include "kernel/output.h"
#include "kernel/operators.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/string.h"
#include "kernel/file.h"
#include "kernel/debug.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Mvc\View
 *
 * Phalcon\Mvc\View is a class for working with the "view" portion of the model-view-controller pattern.
 * That is, it exists to help keep the view script separate from the model and controller scripts.
 * It provides a system of helpers, output filters, and variable escaping.
 *
 * <code>
 * //Setting views directory
 * $view = new Phalcon\Mvc\View();
 * $view->setViewsDir('app/views/');
 *
 * $view->start();
 * //Shows recent posts view (app/views/posts/recent.phtml)
 * $view->render('posts', 'recent');
 * $view->finish();
 *
 * //Printing views output
 * echo $view->getContent();
 * </code>
 */
zend_class_entry *phalcon_mvc_view_ce;

PHP_METHOD(Phalcon_Mvc_View, __construct);
PHP_METHOD(Phalcon_Mvc_View, setViewsDir);
PHP_METHOD(Phalcon_Mvc_View, getViewsDir);
PHP_METHOD(Phalcon_Mvc_View, setLayoutsDir);
PHP_METHOD(Phalcon_Mvc_View, getLayoutsDir);
PHP_METHOD(Phalcon_Mvc_View, setPartialsDir);
PHP_METHOD(Phalcon_Mvc_View, getPartialsDir);
PHP_METHOD(Phalcon_Mvc_View, setBasePath);
PHP_METHOD(Phalcon_Mvc_View, getBasePath);
PHP_METHOD(Phalcon_Mvc_View, getCurrentRenderLevel);
PHP_METHOD(Phalcon_Mvc_View, getRenderLevel);
PHP_METHOD(Phalcon_Mvc_View, setRenderLevel);
PHP_METHOD(Phalcon_Mvc_View, disableLevel);
PHP_METHOD(Phalcon_Mvc_View, getDisabledLevels);
PHP_METHOD(Phalcon_Mvc_View, setMainView);
PHP_METHOD(Phalcon_Mvc_View, getMainView);
PHP_METHOD(Phalcon_Mvc_View, setLayout);
PHP_METHOD(Phalcon_Mvc_View, getLayout);
PHP_METHOD(Phalcon_Mvc_View, setTemplateBefore);
PHP_METHOD(Phalcon_Mvc_View, cleanTemplateBefore);
PHP_METHOD(Phalcon_Mvc_View, setTemplateAfter);
PHP_METHOD(Phalcon_Mvc_View, cleanTemplateAfter);
PHP_METHOD(Phalcon_Mvc_View, setParamToView);
PHP_METHOD(Phalcon_Mvc_View, setVars);
PHP_METHOD(Phalcon_Mvc_View, setVar);
PHP_METHOD(Phalcon_Mvc_View, getVar);
PHP_METHOD(Phalcon_Mvc_View, getParamsToView);
PHP_METHOD(Phalcon_Mvc_View, setControllerName);
PHP_METHOD(Phalcon_Mvc_View, getControllerName);
PHP_METHOD(Phalcon_Mvc_View, setActionName);
PHP_METHOD(Phalcon_Mvc_View, getActionName);
PHP_METHOD(Phalcon_Mvc_View, setParams);
PHP_METHOD(Phalcon_Mvc_View, getParams);
PHP_METHOD(Phalcon_Mvc_View, setNamespaceName);
PHP_METHOD(Phalcon_Mvc_View, getNamespaceName);
PHP_METHOD(Phalcon_Mvc_View, start);
PHP_METHOD(Phalcon_Mvc_View, _loadTemplateEngines);
PHP_METHOD(Phalcon_Mvc_View, _engineRender);
PHP_METHOD(Phalcon_Mvc_View, registerEngines);
PHP_METHOD(Phalcon_Mvc_View, getRegisteredEngines);
PHP_METHOD(Phalcon_Mvc_View, getEngines);
PHP_METHOD(Phalcon_Mvc_View, exists);
PHP_METHOD(Phalcon_Mvc_View, render);
PHP_METHOD(Phalcon_Mvc_View, pick);
PHP_METHOD(Phalcon_Mvc_View, partial);
PHP_METHOD(Phalcon_Mvc_View, getRender);
PHP_METHOD(Phalcon_Mvc_View, finish);
PHP_METHOD(Phalcon_Mvc_View, _createCache);
PHP_METHOD(Phalcon_Mvc_View, isCaching);
PHP_METHOD(Phalcon_Mvc_View, getCache);
PHP_METHOD(Phalcon_Mvc_View, cache);
PHP_METHOD(Phalcon_Mvc_View, setContent);
PHP_METHOD(Phalcon_Mvc_View, getContent);
PHP_METHOD(Phalcon_Mvc_View, getActiveRenderPath);
PHP_METHOD(Phalcon_Mvc_View, disable);
PHP_METHOD(Phalcon_Mvc_View, enable);
PHP_METHOD(Phalcon_Mvc_View, isDisabled);
PHP_METHOD(Phalcon_Mvc_View, reset);
PHP_METHOD(Phalcon_Mvc_View, __set);
PHP_METHOD(Phalcon_Mvc_View, __get);
PHP_METHOD(Phalcon_Mvc_View, __isset);
PHP_METHOD(Phalcon_Mvc_View, enableNamespaceView);
PHP_METHOD(Phalcon_Mvc_View, disableNamespaceView);
PHP_METHOD(Phalcon_Mvc_View, enableLowerCase);
PHP_METHOD(Phalcon_Mvc_View, disableLowerCase);
PHP_METHOD(Phalcon_Mvc_View, setConverter);
PHP_METHOD(Phalcon_Mvc_View, getConverter);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_disablelevel, 0, 0, 1)
	ZEND_ARG_INFO(0, level)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_setvars, 0, 0, 1)
	ZEND_ARG_INFO(0, params)
	ZEND_ARG_INFO(0, merge)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_getvar, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_exists, 0, 0, 1)
	ZEND_ARG_INFO(0, view)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_getrender, 0, 0, 2)
	ZEND_ARG_INFO(0, controllerName)
	ZEND_ARG_INFO(0, actionName)
	ZEND_ARG_INFO(0, params)
	ZEND_ARG_INFO(0, configCallback)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_view_method_entry[] = {
	PHP_ME(Phalcon_Mvc_View, __construct, arginfo_phalcon_mvc_view___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_View, setViewsDir, arginfo_phalcon_mvc_viewinterface_setviewsdir, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getViewsDir, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setLayoutsDir, arginfo_phalcon_mvc_viewinterface_setlayoutsdir, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getLayoutsDir, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setPartialsDir, arginfo_phalcon_mvc_viewinterface_setpartialsdir, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getPartialsDir, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setBasePath, arginfo_phalcon_mvc_viewinterface_setbasepath, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getBasePath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getCurrentRenderLevel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getRenderLevel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setRenderLevel, arginfo_phalcon_mvc_viewinterface_setrenderlevel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, disableLevel, arginfo_phalcon_mvc_view_disablelevel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getDisabledLevels, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setMainView, arginfo_phalcon_mvc_viewinterface_setmainview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getMainView, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setLayout, arginfo_phalcon_mvc_viewinterface_setlayout, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getLayout, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setTemplateBefore, arginfo_phalcon_mvc_viewinterface_settemplatebefore, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, cleanTemplateBefore, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setTemplateAfter, arginfo_phalcon_mvc_viewinterface_settemplateafter, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, cleanTemplateAfter, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setParamToView, arginfo_phalcon_mvc_viewinterface_setparamtoview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setVars, arginfo_phalcon_mvc_view_setvars, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setVar, arginfo_phalcon_mvc_viewinterface_setvar, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getVar, arginfo_phalcon_mvc_view_getvar, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getParamsToView, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setControllerName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getControllerName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setActionName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getActionName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setNamespaceName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getNamespaceName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, _loadTemplateEngines, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_View, _engineRender, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_View, registerEngines, arginfo_phalcon_mvc_viewinterface_registerengines, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getRegisteredEngines, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getEngines, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, exists, arginfo_phalcon_mvc_view_exists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, render, arginfo_phalcon_mvc_viewinterface_render, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, pick, arginfo_phalcon_mvc_viewinterface_pick, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, partial, arginfo_phalcon_mvc_viewinterface_partial, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getRender, arginfo_phalcon_mvc_view_getrender, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, finish, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, _createCache, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_View, isCaching, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getCache, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, cache, arginfo_phalcon_mvc_viewinterface_cache, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setContent, arginfo_phalcon_mvc_viewinterface_setcontent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getContent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getActiveRenderPath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, disable, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, enable, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, isDisabled, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, reset, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, __set, arginfo___set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, __get, arginfo___get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, __isset, arginfo___isset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, enableNamespaceView, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, disableNamespaceView, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, enableLowerCase, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, disableLowerCase, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, setConverter, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View, getConverter, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\View initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_View){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc, View, mvc_view, phalcon_di_injectable_ce, phalcon_mvc_view_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_view_ce, SL("_options"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(phalcon_mvc_view_ce, SL("_basePath"), "", ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(phalcon_mvc_view_ce, SL("_content"), "", ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(phalcon_mvc_view_ce, SL("_renderLevel"), 6, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(phalcon_mvc_view_ce, SL("_currentRenderLevel"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_disabledLevels"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_viewParams"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_layout"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(phalcon_mvc_view_ce, SL("_layoutsDir"), "", ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(phalcon_mvc_view_ce, SL("_partialsDir"), "", ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_enableLayoutsAbsolutePath"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_enablePartialsAbsolutePath"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_viewsDir"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_enableNamespaceView"), 1, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_templatesBefore"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_templatesAfter"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_engines"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_registeredEngines"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(phalcon_mvc_view_ce, SL("_mainView"), "index", ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_controllerName"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_namespaceName"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_actionName"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_params"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_pickView"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_cache"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(phalcon_mvc_view_ce, SL("_cacheLevel"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_cacheMode"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_activeRenderPath"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_disabled"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_bool(phalcon_mvc_view_ce, SL("_lowerCase"), 1, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_view_ce, SL("_converters"), ZEND_ACC_PROTECTED TSRMLS_CC);

	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_MAIN_LAYOUT"), 6 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_AFTER_TEMPLATE"), 5 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_NAMESPACE"), 4 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_CONTROLLER"), 3 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_LAYOUT"), 3 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_BEFORE_TEMPLATE"), 2 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_ACTION_VIEW"), 1 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_view_ce, SL("LEVEL_NO_RENDER"), 0 TSRMLS_CC);

	zend_declare_class_constant_bool(phalcon_mvc_view_ce, SL("CACHE_MODE_NONE"), 0 TSRMLS_CC);
	zend_declare_class_constant_bool(phalcon_mvc_view_ce, SL("CACHE_MODE_INVERSE"), 1 TSRMLS_CC);

	zend_class_implements(phalcon_mvc_view_ce TSRMLS_CC, 1, phalcon_mvc_viewinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\View constructor
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_View, __construct){

	zval *options = NULL;

	phalcon_fetch_params(0, 0, 1, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		phalcon_update_property_this(this_ptr, SL("_options"), options TSRMLS_CC);
	}

	phalcon_update_property_empty_array(this_ptr, SL("_converters") TSRMLS_CC);
}

/**
 * Sets views directory. Depending of your platform, always add a trailing slash or backslash
 *
 * @param string $viewsDir
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setViewsDir){

	zval **views_dir;

	phalcon_fetch_params_ex(1, 0, &views_dir);
	phalcon_add_trailing_slash(views_dir);
	phalcon_update_property_this(this_ptr, SL("_viewsDir"), *views_dir TSRMLS_CC);

	RETURN_THISW();
}

/**
 * Gets views directory
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getViewsDir){


	RETURN_MEMBER(this_ptr, "_viewsDir");
}

/**
 * Sets the layouts sub-directory. Must be a directory under the views directory. Depending of your platform, always add a trailing slash or backslash
 *
 *<code>
 * $view->setLayoutsDir('../common/layouts/');
 *</code>
 *
 * @param string $layoutsDir
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setLayoutsDir){

	zval **layouts_dir, **absolute_path = NULL;
	int absolute = 0;

	phalcon_fetch_params_ex(1, 1, &layouts_dir, &absolute_path);
	phalcon_add_trailing_slash(layouts_dir);
	absolute = absolute_path ? zend_is_true(*absolute_path) : 0;
	phalcon_update_property_this(this_ptr, SL("_layoutsDir"), *layouts_dir TSRMLS_CC);
	phalcon_update_property_bool(this_ptr, SL("_enableLayoutsAbsolutePath"), absolute TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Gets the current layouts sub-directory
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getLayoutsDir){


	RETURN_MEMBER(this_ptr, "_layoutsDir");
}

/**
 * Sets a partials sub-directory. Must be a directory under the views directory. Depending of your platform, always add a trailing slash or backslash
 *
 *<code>
 * $view->setPartialsDir('../common/partials/');
 *</code>
 *
 * @param string $partialsDir
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setPartialsDir){

	zval **partials_dir, **absolute_path = NULL;
	int absolute = 0;

	phalcon_fetch_params_ex(1, 1, &partials_dir, &absolute_path);
	phalcon_add_trailing_slash(partials_dir);
	absolute = absolute_path ? zend_is_true(*absolute_path) : 0;
	phalcon_update_property_this(this_ptr, SL("_partialsDir"), *partials_dir TSRMLS_CC);
	phalcon_update_property_bool(this_ptr, SL("_enablePartialsAbsolutePath"), absolute TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Gets the current partials sub-directory
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getPartialsDir){


	RETURN_MEMBER(this_ptr, "_partialsDir");
}

/**
 * Sets base path. Depending of your platform, always add a trailing slash or backslash
 *
 * <code>
 * 	$view->setBasePath(__DIR__ . '/');
 * </code>
 *
 * @param string|array $basePath
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setBasePath){

	zval *base_path, *base_paths, *path = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &base_path);

	if (Z_TYPE_P(base_path) == IS_ARRAY) {
		PHALCON_INIT_VAR(base_paths);
		array_init(base_paths);

		phalcon_is_iterable(base_path, &ah0, &hp0, 0, 0);

		while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {

			PHALCON_GET_HVALUE(path);

			phalcon_add_trailing_slash(&path);

			phalcon_array_append(&base_paths, path, PH_COPY);

			zend_hash_move_forward_ex(ah0, &hp0);
		}

		phalcon_update_property_this(this_ptr, SL("_basePath"), base_paths TSRMLS_CC);
	} else {
		phalcon_add_trailing_slash(&base_path);
		phalcon_update_property_this(this_ptr, SL("_basePath"), base_path TSRMLS_CC);
	}

	RETURN_THIS();
}

/**
 * Gets base path
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getBasePath){

	RETURN_MEMBER(this_ptr, "_basePath");
}

/**
 * Returns the render level for the view
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_View, getCurrentRenderLevel) {

	RETURN_MEMBER(getThis(), "_currentRenderLevel");
}

/**
 * Returns the render level for the view
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_View, getRenderLevel) {

	RETURN_MEMBER(getThis(), "_renderLevel");
}

/**
 * Sets the render level for the view
 *
 * <code>
 * 	//Render the view related to the controller only
 * 	$this->view->setRenderLevel(View::LEVEL_LAYOUT);
 * </code>
 *
 * @param string $level
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setRenderLevel){

	zval *level;

	phalcon_fetch_params(0, 1, 0, &level);

	phalcon_update_property_this(this_ptr, SL("_renderLevel"), level TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Disables a specific level of rendering
 *
 *<code>
 * //Render all levels except ACTION level
 * $this->view->disableLevel(View::LEVEL_ACTION_VIEW);
 *</code>
 *
 * @param int|array $level
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, disableLevel){

	zval *level;

	phalcon_fetch_params(0, 1, 0, &level);

	if (Z_TYPE_P(level) == IS_ARRAY) { 
		phalcon_update_property_this(this_ptr, SL("_disabledLevels"), level TSRMLS_CC);
	} else {
		phalcon_update_property_array(this_ptr, SL("_disabledLevels"), level, PHALCON_GLOBAL(z_true) TSRMLS_CC);
	}

	RETURN_THISW();
}

/**
 * Returns an array with disabled render levels
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View, getDisabledLevels) {

	RETURN_MEMBER(getThis(), "_disabledLevels");
}

/**
 * Sets default view name. Must be a file without extension in the views directory
 *
 * <code>
 * 	//Renders as main view views-dir/base.phtml
 * 	$this->view->setMainView('base');
 * </code>
 *
 * @param string $viewPath
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setMainView){

	zval *view_path;

	phalcon_fetch_params(0, 1, 0, &view_path);

	phalcon_update_property_this(this_ptr, SL("_mainView"), view_path TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Returns the name of the main view
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getMainView){


	RETURN_MEMBER(this_ptr, "_mainView");
}

/**
 * Change the layout to be used instead of using the name of the latest controller name
 *
 * <code>
 * 	$this->view->setLayout('main');
 * </code>
 *
 * @param string $layout
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setLayout){

	zval *layout;

	phalcon_fetch_params(0, 1, 0, &layout);

	phalcon_update_property_this(this_ptr, SL("_layout"), layout TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Returns the name of the main view
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getLayout){


	RETURN_MEMBER(this_ptr, "_layout");
}

/**
 * Appends template before controller layout
 *
 * @param string|array $templateBefore
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setTemplateBefore){

	zval *template_before, *array_template;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &template_before);

	if (Z_TYPE_P(template_before) != IS_ARRAY) { 
		PHALCON_INIT_VAR(array_template);
		array_init_size(array_template, 1);
		phalcon_array_append(&array_template, template_before, PH_COPY);
		phalcon_update_property_this(this_ptr, SL("_templatesBefore"), array_template TSRMLS_CC);
	} else {
		phalcon_update_property_this(this_ptr, SL("_templatesBefore"), template_before TSRMLS_CC);
	}

	RETURN_THIS();
}

/**
 * Resets any template before layouts
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, cleanTemplateBefore){


	phalcon_update_property_null(this_ptr, SL("_templatesBefore") TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Appends template after controller layout
 *
 * @param string|array $templateAfter
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setTemplateAfter){

	zval *template_after, *array_template;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &template_after);

	if (Z_TYPE_P(template_after) != IS_ARRAY) { 
		PHALCON_INIT_VAR(array_template);
		array_init_size(array_template, 1);
		phalcon_array_append(&array_template, template_after, PH_COPY);
		phalcon_update_property_this(this_ptr, SL("_templatesAfter"), array_template TSRMLS_CC);
	} else {
		phalcon_update_property_this(this_ptr, SL("_templatesAfter"), template_after TSRMLS_CC);
	}

	RETURN_THIS();
}

/**
 * Resets any template after layouts
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, cleanTemplateAfter){


	phalcon_update_property_null(this_ptr, SL("_templatesAfter") TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Adds parameters to views (alias of setVar)
 *
 *<code>
 *	$this->view->setParamToView('products', $products);
 *</code>
 *
 * @param string $key
 * @param mixed $value
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setParamToView){

	zval *key, *value;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	phalcon_update_property_array(this_ptr, SL("_viewParams"), key, value TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Set all the render params
 *
 *<code>
 *	$this->view->setVars(array('products' => $products));
 *</code>
 *
 * @param array $params
 * @param boolean $merge
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setVars){

	zval *params, *merge = NULL, *view_params, *merged_params = NULL;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 1, &params, &merge);

	if (!merge) {
		merge = PHALCON_GLOBAL(z_true);
	}

	if (Z_TYPE_P(params) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_view_exception_ce, "The render parameters must be an array");
		return;
	}
	if (zend_is_true(merge)) {

		PHALCON_OBS_VAR(view_params);
		phalcon_read_property_this(&view_params, this_ptr, SL("_viewParams"), PH_NOISY TSRMLS_CC);
		if (Z_TYPE_P(view_params) == IS_ARRAY) { 
			PHALCON_INIT_VAR(merged_params);
			phalcon_fast_array_merge(merged_params, &view_params, &params TSRMLS_CC);
		} else {
			PHALCON_CPY_WRT(merged_params, params);
		}

		phalcon_update_property_this(this_ptr, SL("_viewParams"), merged_params TSRMLS_CC);
	} else {
		phalcon_update_property_this(this_ptr, SL("_viewParams"), params TSRMLS_CC);
	}

	RETURN_THIS();
}

/**
 * Set a single view parameter
 *
 *<code>
 *	$this->view->setVar('products', $products);
 *</code>
 *
 * @param string $key
 * @param mixed $value
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setVar){

	zval *key, *value;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	phalcon_update_property_array(this_ptr, SL("_viewParams"), key, value TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Returns a parameter previously set in the view
 *
 * @param string $key
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_View, getVar){

	zval *key, *params, *value;

	phalcon_fetch_params(0, 1, 0, &key);

	params = phalcon_fetch_nproperty_this(this_ptr, SL("_viewParams"), PH_NOISY TSRMLS_CC);
	if (phalcon_array_isset_fetch(&value, params, key)) {
		RETURN_ZVAL(value, 1, 0);
	}

	RETURN_NULL();
}

/**
 * Returns parameters to views
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View, getParamsToView){


	RETURN_MEMBER(this_ptr, "_viewParams");
}

/**
 * Sets the controller name to be view
 *
 * @param string $controllerName
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setControllerName){

	zval *controller_name;

	phalcon_fetch_params(0, 1, 0, &controller_name);

	phalcon_update_property_this(this_ptr, SL("_controllerName"), controller_name TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Gets the name of the controller rendered
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getControllerName){


	RETURN_MEMBER(this_ptr, "_controllerName");
}

/**
 * Sets the action name to be view
 *
 * @param string $actionName
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setActionName){

	zval *action_name;

	phalcon_fetch_params(0, 1, 0, &action_name);

	phalcon_update_property_this(this_ptr, SL("_actionName"), action_name TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Gets the name of the action rendered
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getActionName){


	RETURN_MEMBER(this_ptr, "_actionName");
}

/**
 * Sets the extra parameters to be view
 *
 * @param array $params
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setParams){

	zval *params;

	phalcon_fetch_params(0, 1, 0, &params);

	phalcon_update_property_this(this_ptr, SL("_params"), params TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Gets extra parameters of the action rendered
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View, getParams){


	RETURN_MEMBER(this_ptr, "_params");
}

PHP_METHOD(Phalcon_Mvc_View, setNamespaceName){

	zval *namespace_name;

	phalcon_fetch_params(0, 1, 0, &namespace_name);

	phalcon_update_property_this(this_ptr, SL("_namespaceName"), namespace_name TSRMLS_CC);
	RETURN_THISW();
}

PHP_METHOD(Phalcon_Mvc_View, getNamespaceName){


	RETURN_MEMBER(this_ptr, "_namespaceName");
}

/**
 * Starts rendering process enabling the output buffering
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, start){

	phalcon_update_property_null(this_ptr, SL("_content") TSRMLS_CC);
	phalcon_ob_start(TSRMLS_C);
	RETURN_THISW();
}

/**
 * Loads registered template engines, if none is registered it will use Phalcon\Mvc\View\Engine\Php
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View, _loadTemplateEngines){

	zval *engines = NULL, *dependency_injector = NULL, *registered_engines;
	zval *php_engine, *arguments, *engine_service = NULL;
	zval *extension = NULL, *engine_object = NULL, *exception_message = NULL;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(engines);
	phalcon_read_property_this(&engines, this_ptr, SL("_engines"), PH_NOISY TSRMLS_CC);

	/** 
	 * If the engines aren't initialized 'engines' is false
	 */
	if (PHALCON_IS_FALSE(engines)) {
		PHALCON_CALL_METHOD(&dependency_injector, this_ptr, "getdi");

		PHALCON_INIT_NVAR(engines);
		array_init(engines);

		PHALCON_OBS_VAR(registered_engines);
		phalcon_read_property_this(&registered_engines, this_ptr, SL("_registeredEngines"), PH_NOISY TSRMLS_CC);
		if (Z_TYPE_P(registered_engines) != IS_ARRAY) { 
			/** 
			 * We use Phalcon\Mvc\View\Engine\Php as default
			 */
			PHALCON_INIT_VAR(php_engine);
			object_init_ex(php_engine, phalcon_mvc_view_engine_php_ce);
			PHALCON_CALL_METHOD(NULL, php_engine, "__construct", this_ptr, dependency_injector);

			phalcon_array_update_string(&engines, SL(".phtml"), php_engine, PH_COPY);
		} else {
			PHALCON_INIT_VAR(arguments);
			array_init_size(arguments, 2);
			phalcon_array_append(&arguments, this_ptr, PH_COPY);
			phalcon_array_append(&arguments, dependency_injector, PH_COPY);

			phalcon_is_iterable(registered_engines, &ah0, &hp0, 0, 0);

			while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {

				PHALCON_GET_HKEY(extension, ah0, hp0);
				PHALCON_GET_HVALUE(engine_service);

				if (Z_TYPE_P(engine_service) == IS_OBJECT) {

					/** 
					 * Engine can be a closure
					 */
					if (instanceof_function(Z_OBJCE_P(engine_service), zend_ce_closure TSRMLS_CC)) {
						PHALCON_INIT_NVAR(engine_object); /**/
						PHALCON_CALL_USER_FUNC_ARRAY(engine_object, engine_service, arguments);
					} else {
						PHALCON_CPY_WRT(engine_object, engine_service);
					}
				} else {
					/** 
					 * Engine can be a string representing a service in the DI
					 */
					if (Z_TYPE_P(engine_service) == IS_STRING) {
						PHALCON_CALL_METHOD(&engine_object, dependency_injector, "getshared", engine_service, arguments);
						PHALCON_VERIFY_INTERFACE(engine_object, phalcon_mvc_view_engineinterface_ce);
					} else {
						PHALCON_INIT_NVAR(exception_message);
						PHALCON_CONCAT_SV(exception_message, "Invalid template engine registration for extension: ", extension);
						PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_view_exception_ce, exception_message);
						return;
					}
				}
				phalcon_array_update_zval(&engines, extension, engine_object, PH_COPY);

				zend_hash_move_forward_ex(ah0, &hp0);
			}

		}

		phalcon_update_property_this(this_ptr, SL("_engines"), engines TSRMLS_CC);
	} else {
		PHALCON_OBS_NVAR(engines);
		phalcon_read_property_this(&engines, this_ptr, SL("_engines"), PH_NOISY TSRMLS_CC);
	}

	RETURN_CCTOR(engines);
}

/**
 * Checks whether view exists on registered extensions and render it
 *
 * @param array $engines
 * @param string $viewPath
 * @param boolean $silence
 * @param boolean $mustClean
 */
PHP_METHOD(Phalcon_Mvc_View, _engineRender){

	zval *engines, *view_path, *silence, *must_clean, *absolute_path = NULL, *debug_message = NULL;
	zval *cache = NULL, *not_exists = NULL, *views_dir, *base_path;
	zval *path = NULL, *dir = NULL, *views_dir_paths, *views_dir_path = NULL, *render_level, *cache_level, *cache_mode;
	zval *key = NULL, *lifetime = NULL, *view_options;
	zval *cache_options, *cached_view = NULL;
	zval *view_params, *events_manager, *engine = NULL;
	zval *extension = NULL, *view_engine_path = NULL, *event_name = NULL;
	zval *status = NULL, *exception_message;
	HashTable *ah0, *ah01, *ah1, *ah2;
	HashPosition hp0, hp01, hp1, hp2;
	zval **hd;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 4, 1, &engines, &view_path, &silence, &must_clean, &absolute_path);

	if (absolute_path == NULL) {
		absolute_path = PHALCON_GLOBAL(z_false);
	}

	/** 
	 * Start the cache if there is a cache level enabled
	 */
	PHALCON_OBS_VAR(cache_level);
	phalcon_read_property_this(&cache_level, this_ptr, SL("_cacheLevel"), PH_NOISY TSRMLS_CC);

	if (zend_is_true(cache_level)) {		
		PHALCON_OBS_VAR(render_level);
		phalcon_read_property_this(&render_level, this_ptr, SL("_currentRenderLevel"), PH_NOISY TSRMLS_CC);

		PHALCON_OBS_VAR(cache_mode);
		phalcon_read_property_this(&cache_mode, this_ptr, SL("_cacheMode"), PH_NOISY TSRMLS_CC);

		if (PHALCON_IS_TRUE(cache_mode)) {
			if (PHALCON_LE(render_level, cache_level)) {
				PHALCON_CALL_METHOD(&cache, this_ptr, "getcache");
			} else {
				PHALCON_INIT_VAR(cache);
			}	
		} else {
			if (PHALCON_GE(render_level, cache_level)) {
				PHALCON_CALL_METHOD(&cache, this_ptr, "getcache");
			} else {
				PHALCON_INIT_VAR(cache);
			}
		}
	} else {
		PHALCON_INIT_VAR(cache);
	}

	PHALCON_INIT_VAR(not_exists);
	ZVAL_TRUE(not_exists);

	PHALCON_INIT_VAR(views_dir_paths);
	array_init(views_dir_paths);

	if (zend_is_true(absolute_path)) {
		phalcon_array_append(&views_dir_paths, view_path, PH_COPY);
	} else {
		PHALCON_OBS_VAR(base_path);
		phalcon_read_property_this(&base_path, this_ptr, SL("_basePath"), PH_NOISY TSRMLS_CC);

		PHALCON_OBS_VAR(views_dir);
		phalcon_read_property_this(&views_dir, this_ptr, SL("_viewsDir"), PH_NOISY TSRMLS_CC);

		if (Z_TYPE_P(base_path) == IS_ARRAY) {
			phalcon_is_iterable(base_path, &ah0, &hp0, 0, 0);
			while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {
				PHALCON_GET_HVALUE(path);

				if (Z_TYPE_P(views_dir) == IS_ARRAY) {
					phalcon_is_iterable(views_dir, &ah01, &hp01, 0, 0);
					while (zend_hash_get_current_data_ex(ah01, (void**) &hd, &hp01) == SUCCESS) {
						PHALCON_GET_HVALUE(dir);

						PHALCON_INIT_NVAR(views_dir_path);
						PHALCON_CONCAT_VVV(views_dir_path, path, dir, view_path);
						phalcon_array_append(&views_dir_paths, views_dir_path, PH_COPY);

						zend_hash_move_forward_ex(ah01, &hp01);
					}
				} else {
					PHALCON_INIT_NVAR(views_dir_path);
					PHALCON_CONCAT_VVV(views_dir_path, path, views_dir, view_path);
					phalcon_array_append(&views_dir_paths, views_dir_path, PH_COPY);
				}

				zend_hash_move_forward_ex(ah0, &hp0);
			}
		} else {
			if (Z_TYPE_P(views_dir) == IS_ARRAY) {
					phalcon_is_iterable(views_dir, &ah01, &hp01, 0, 0);
					while (zend_hash_get_current_data_ex(ah01, (void**) &hd, &hp01) == SUCCESS) {
						PHALCON_GET_HVALUE(dir);

						PHALCON_INIT_NVAR(views_dir_path);
						PHALCON_CONCAT_VVV(views_dir_path, base_path, dir, view_path);
						phalcon_array_append(&views_dir_paths, views_dir_path, PH_COPY);

						zend_hash_move_forward_ex(ah01, &hp01);
					}
			} else {
				PHALCON_INIT_VAR(views_dir_path);
				PHALCON_CONCAT_VVV(views_dir_path, base_path, views_dir, view_path);
				phalcon_array_append(&views_dir_paths, views_dir_path, PH_COPY);
			}
		}
	}

	if (Z_TYPE_P(cache) == IS_OBJECT) {

		/** 
		 * Check if the cache is started, the first time a cache is started we start the
		 * cache
		 */
		PHALCON_INIT_VAR(key);
		PHALCON_INIT_VAR(lifetime);
		PHALCON_OBS_VAR(view_options);

		phalcon_read_property_this(&view_options, this_ptr, SL("_options"), PH_NOISY TSRMLS_CC);

		/** 
		 * Check if the user has defined a different options to the default
		 */
		if (Z_TYPE_P(view_options) == IS_ARRAY) { 
			if (phalcon_array_isset_string(view_options, SS("cache"))) {

				PHALCON_OBS_VAR(cache_options);
				phalcon_array_fetch_string(&cache_options, view_options, SL("cache"), PH_NOISY);
				if (Z_TYPE_P(cache_options) == IS_ARRAY) { 
					if (phalcon_array_isset_string(cache_options, SS("key"))) {
						PHALCON_OBS_NVAR(key);
						phalcon_array_fetch_string(&key, cache_options, SL("key"), PH_NOISY);
					}
					if (phalcon_array_isset_string(cache_options, SS("lifetime"))) {
						PHALCON_OBS_NVAR(lifetime);
						phalcon_array_fetch_string(&lifetime, cache_options, SL("lifetime"), PH_NOISY);
					}
				}
			}
		}

		/** 
		 * If a cache key is not set we create one using a md5
		 */
		if (Z_TYPE_P(key) == IS_NULL) {
			PHALCON_INIT_NVAR(key);
			phalcon_md5(key, view_path);
		}

		/** 
		 * We start the cache using the key set
		 */
		phalcon_ob_clean(TSRMLS_C);
		PHALCON_CALL_METHOD(&cached_view, cache, "start", key, lifetime);
		if (Z_TYPE_P(cached_view) != IS_NULL) {
			phalcon_update_property_this(this_ptr, SL("_content"), cached_view TSRMLS_CC);
			RETURN_MM_NULL();
		}
	}

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_INIT_NVAR(debug_message);
		PHALCON_CONCAT_SV(debug_message, "Render View: ", view_path);
		phalcon_debug_print_r(debug_message TSRMLS_CC);
	}

	PHALCON_OBS_VAR(view_params);
	phalcon_read_property_this(&view_params, this_ptr, SL("_viewParams"), PH_NOISY TSRMLS_CC);

	PHALCON_OBS_VAR(events_manager);
	phalcon_read_property_this(&events_manager, this_ptr, SL("_eventsManager"), PH_NOISY TSRMLS_CC);

	PHALCON_INIT_VAR(view_engine_path);

	/** 
	 * Views are rendered in each engine
	 */
	phalcon_is_iterable(engines, &ah1, &hp1, 0, 0);

	while (zend_hash_get_current_data_ex(ah1, (void**) &hd, &hp1) == SUCCESS) {

		PHALCON_GET_HKEY(extension, ah1, hp1);
		PHALCON_GET_HVALUE(engine);

		phalcon_is_iterable(views_dir_paths, &ah2, &hp2, 0, 0);
		
		while (zend_hash_get_current_data_ex(ah2, (void**) &hd, &hp2) == SUCCESS) {

			PHALCON_GET_HVALUE(path);

			PHALCON_INIT_NVAR(view_engine_path);
			PHALCON_CONCAT_VV(view_engine_path, path, extension);

			if (phalcon_file_exists(view_engine_path TSRMLS_CC) == SUCCESS) {

				if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
					PHALCON_INIT_NVAR(debug_message);
					PHALCON_CONCAT_SV(debug_message, "--Found: ", view_engine_path);
					phalcon_debug_print_r(debug_message TSRMLS_CC);
				}

				/** 
				 * Call beforeRenderView if there is a events manager available
				 */
				if (Z_TYPE_P(events_manager) == IS_OBJECT) {
					phalcon_update_property_this(this_ptr, SL("_activeRenderPath"), view_engine_path TSRMLS_CC);
		
					PHALCON_INIT_NVAR(event_name);
					ZVAL_STRING(event_name, "view:beforeRenderView", 1);
		
					PHALCON_CALL_METHOD(&status, events_manager, "fire", event_name, this_ptr, view_engine_path);
					if (PHALCON_IS_FALSE(status)) {
						zend_hash_move_forward_ex(ah0, &hp0);
						continue;
					}
				}

				PHALCON_CALL_METHOD(NULL, engine, "render", view_engine_path, view_params, must_clean);
		
				/** 
				 * Call afterRenderView if there is a events manager available
				 */
				PHALCON_INIT_NVAR(not_exists);
				ZVAL_FALSE(not_exists);

				if (Z_TYPE_P(events_manager) == IS_OBJECT) {
					PHALCON_INIT_NVAR(event_name);
					ZVAL_STRING(event_name, "view:afterRenderView", 1);
					PHALCON_CALL_METHOD(NULL, events_manager, "fire", event_name, this_ptr);
				}

				break;
			} else if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
				PHALCON_INIT_NVAR(debug_message);
				PHALCON_CONCAT_SV(debug_message, "--Not Found: ", view_engine_path);
				phalcon_debug_print_r(debug_message TSRMLS_CC);
			}

			zend_hash_move_forward_ex(ah2, &hp2);
		}

		if (!zend_is_true(not_exists)) {
			break;
		}

		zend_hash_move_forward_ex(ah1, &hp1);
	}

	if (PHALCON_IS_TRUE(not_exists)) {

		/** 
		 * Notify about not found views
		 */
		if (Z_TYPE_P(events_manager) == IS_OBJECT) {
			phalcon_update_property_this(this_ptr, SL("_activeRenderPath"), view_engine_path TSRMLS_CC);

			PHALCON_INIT_NVAR(event_name);
			ZVAL_STRING(event_name, "view:notFoundView", 1);
			PHALCON_CALL_METHOD(NULL, events_manager, "fire", event_name, this_ptr);
		}
		if (!zend_is_true(silence)) {
			PHALCON_INIT_VAR(exception_message);
			PHALCON_CONCAT_SVS(exception_message, "View '", views_dir_path, "' was not found in the views directory");
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_view_exception_ce, exception_message);
			return;
		}
	}

	/** 
	 * Store the data in the cache
	 */
	if (Z_TYPE_P(cache) == IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, cache, "save");
	}

	PHALCON_MM_RESTORE();
}

/**
 * Register templating engines
 *
 *<code>
 *$this->view->registerEngines(array(
 *  ".phtml" => "Phalcon\Mvc\View\Engine\Php",
 *  ".volt" => "Phalcon\Mvc\View\Engine\Volt",
 *  ".mhtml" => "MyCustomEngine"
 *));
 *</code>
 *
 * @param array $engines
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, registerEngines){

	zval *engines;

	phalcon_fetch_params(0, 1, 0, &engines);

	if (Z_TYPE_P(engines) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_view_exception_ce, "Engines to register must be an array");
		return;
	}
	phalcon_update_property_this(this_ptr, SL("_registeredEngines"), engines TSRMLS_CC);

	RETURN_THISW();
}

/**
 * Returns the registered templating engines
 *
 * @brief array Phalcon\Mvc\View::getRegisteredEngines()
 */
PHP_METHOD(Phalcon_Mvc_View, getRegisteredEngines) {

	RETURN_MEMBER(getThis(), "_registeredEngines")
}

/**
 * Returns the registered templating engines, if none is registered it will use Phalcon\Mvc\View\Engine\Php
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View, getEngines) {

	PHALCON_RETURN_CALL_METHODW(this_ptr, "_loadtemplateengines");
}

PHP_METHOD(Phalcon_Mvc_View, exists) {

	zval **view, *base_dir, *view_dir, *engines;
	HashPosition pos;
	zval *path;
	int exists = 0;

	phalcon_fetch_params_ex(1, 0, &view);
	PHALCON_ENSURE_IS_STRING(view);

	base_dir = phalcon_fetch_nproperty_this(getThis(), SL("_basePath"), PH_NOISY TSRMLS_CC);
	view_dir = phalcon_fetch_nproperty_this(getThis(), SL("_viewsDir"), PH_NOISY TSRMLS_CC);
	engines  = phalcon_fetch_nproperty_this(getThis(), SL("_registeredEngines"), PH_NOISY TSRMLS_CC);

	if (Z_TYPE_P(engines) != IS_ARRAY) {
		MAKE_STD_ZVAL(engines);
		array_init_size(engines, 1);
		add_assoc_stringl_ex(engines, SS(".phtml"), (char*)phalcon_mvc_view_engine_php_ce->name, phalcon_mvc_view_engine_php_ce->name_length, !IS_INTERNED(phalcon_mvc_view_engine_php_ce->name));
		phalcon_update_property_this(getThis(), SL("_registeredEngines"), engines TSRMLS_CC);
		assert(Z_REFCOUNT_P(engines) > 1);
		zval_ptr_dtor(&engines);
	}

	MAKE_STD_ZVAL(path);
	for (
		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(engines), &pos);
		!exists && HASH_KEY_NON_EXISTANT != zend_hash_get_current_key_type_ex(Z_ARRVAL_P(engines), &pos);
		zend_hash_move_forward_ex(Z_ARRVAL_P(engines), &pos)
	) {
		zval ext = phalcon_get_current_key_w(Z_ARRVAL_P(engines), &pos);

		PHALCON_CONCAT_VVVV(path, base_dir, view_dir, *view, &ext);
		exists = (SUCCESS == phalcon_file_exists(path TSRMLS_CC));
		zval_dtor(path);
	}

	ZVAL_NULL(path);
	zval_ptr_dtor(&path);

	RETURN_BOOL(exists);
}

/**
 * Executes render process from dispatching data
 *
 *<code>
 * //Shows recent posts view (app/views/posts/recent.phtml)
 * $view->start()->render('posts', 'recent')->finish();
 *</code>
 *
 * @param string $controllerName
 * @param string $actionName
 * @param array $params
 * @param string $namespace_name
 * @param Phalcon\Mvc\View\ModelInterface $viewModel
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, render){

	zval *controller_name, *action_name, *params = NULL, *namespace_name = NULL, *view_model = NULL;
	zval *disabled, *contents = NULL, *model_content = NULL, *layouts_dir = NULL, *layout, *enable_namespace_view, *lower_case;
	zval *layout_name = NULL, *layout_namespace = NULL, *engines = NULL, *pick_view, *render_view = NULL;
	zval *pick_view_action, *event_name = NULL, *status = NULL;
	zval *silence = NULL, *disabled_levels, *render_level, *enable_layouts_absolute_path;
	zval *templates_before, *template_before = NULL;
	zval *view_temp_path = NULL, *templates_after, *template_after = NULL, *main_view;
	zval *converter_key, *converter = NULL, *parameters = NULL, *lower_controller_name = NULL, *lower_action_name = NULL, *lower_namespace_name = NULL;
	zval *ds, *namespace_separator, *ds_lower_namespace_name = NULL;
	HashTable *ah0, *ah1;
	HashPosition hp0, hp1;
	zval **hd;
	char slash[2] = {DEFAULT_SLASH, 0};
	int use_model = 0;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 2, 3, &controller_name, &action_name, &params, &namespace_name, &view_model);

	if (!params) {
		params = PHALCON_GLOBAL(z_null);
	}

	if (!namespace_name) {
		namespace_name = PHALCON_GLOBAL(z_null);
	}

	if (!view_model) {
		view_model = PHALCON_GLOBAL(z_null);
	} else if (Z_TYPE_P(view_model) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(view_model), phalcon_mvc_view_modelinterface_ce, 1 TSRMLS_CC)) {
		use_model = 1;
	}

	PHALCON_INIT_VAR(ds);
	ZVAL_STRING(ds, slash, 1);

	PHALCON_INIT_VAR(namespace_separator);
	ZVAL_STRING(namespace_separator, "\\", 1);

	phalcon_update_property_this(this_ptr, SL("_currentRenderLevel"), PHALCON_GLOBAL(z_zero) TSRMLS_CC);

	/** 
	 * If the view is disabled we simply update the buffer from any output produced in
	 * the controller
	 */
	PHALCON_OBS_VAR(disabled);
	phalcon_read_property_this(&disabled, this_ptr, SL("_disabled"), PH_NOISY TSRMLS_CC);
	if (PHALCON_IS_NOT_FALSE(disabled)) {
		PHALCON_INIT_VAR(contents);
		phalcon_ob_get_contents(contents TSRMLS_CC);
		phalcon_update_property_this(this_ptr, SL("_content"), contents TSRMLS_CC);
		RETURN_MM_FALSE;
	}

	phalcon_update_property_this(this_ptr, SL("_controllerName"), controller_name TSRMLS_CC);
	phalcon_update_property_this(this_ptr, SL("_actionName"), action_name TSRMLS_CC);
	phalcon_update_property_this(this_ptr, SL("_params"), params TSRMLS_CC);
	phalcon_update_property_this(this_ptr, SL("_namespaceName"), namespace_name TSRMLS_CC);

	PHALCON_INIT_VAR(converter_key);
	ZVAL_STRING(converter_key, "controller", 1);

	PHALCON_CALL_SELF(&converter, "getconverter", converter_key);

	if (phalcon_is_callable(converter TSRMLS_CC)) {
		PHALCON_INIT_NVAR(parameters);
		array_init_size(parameters, 1);
		phalcon_array_append(&parameters, controller_name, PH_COPY);

		PHALCON_SEPARATE_PARAM(controller_name);
		PHALCON_INIT_NVAR(controller_name);
		PHALCON_CALL_USER_FUNC_ARRAY(controller_name, converter, parameters);
	}

	PHALCON_INIT_NVAR(converter_key);
	ZVAL_STRING(converter_key, "action", 1);

	PHALCON_CALL_SELF(&converter, "getconverter", converter_key);

	if (phalcon_is_callable(converter TSRMLS_CC)) {
		PHALCON_INIT_NVAR(parameters);
		array_init_size(parameters, 1);
		phalcon_array_append(&parameters, action_name, PH_COPY);

		PHALCON_SEPARATE_PARAM(action_name);
		PHALCON_INIT_NVAR(action_name);
		PHALCON_CALL_USER_FUNC_ARRAY(action_name, converter, parameters);
	}

	PHALCON_INIT_NVAR(converter_key);
	ZVAL_STRING(converter_key, "namespace", 1);

	PHALCON_CALL_SELF(&converter, "getconverter", converter_key);

	if (phalcon_is_callable(converter TSRMLS_CC)) {
		PHALCON_INIT_NVAR(parameters);
		array_init_size(parameters, 1);
		phalcon_array_append(&parameters, namespace_name, PH_COPY);

		PHALCON_SEPARATE_PARAM(namespace_name);
		PHALCON_INIT_NVAR(namespace_name);
		PHALCON_CALL_USER_FUNC_ARRAY(namespace_name, converter, parameters);
	}

	PHALCON_OBS_VAR(lower_case);
	phalcon_read_property_this(&lower_case, this_ptr, SL("_lowerCase"), PH_NOISY TSRMLS_CC);

	if (zend_is_true(lower_case)) {
		PHALCON_INIT_VAR(lower_controller_name);
		phalcon_fast_strtolower(lower_controller_name, controller_name);

		PHALCON_INIT_VAR(lower_action_name);
		phalcon_fast_strtolower(lower_action_name, action_name);
	} else {
		PHALCON_CPY_WRT_CTOR(lower_controller_name, controller_name);
		PHALCON_CPY_WRT_CTOR(lower_action_name, action_name);
	}

	/** 
	 * Check if there is a layouts directory set
	 */
	PHALCON_OBS_VAR(layouts_dir);
	phalcon_read_property_this(&layouts_dir, this_ptr, SL("_layoutsDir"), PH_NOISY TSRMLS_CC);
	if (!zend_is_true(layouts_dir)) {
		PHALCON_INIT_NVAR(layouts_dir);
		ZVAL_STRING(layouts_dir, "layouts/", 1);
	}

	PHALCON_OBS_VAR(enable_namespace_view);
	phalcon_read_property_this(&enable_namespace_view, this_ptr, SL("_enableNamespaceView"), PH_NOISY TSRMLS_CC);

	if (zend_is_true(enable_namespace_view)) {
		PHALCON_INIT_VAR(lower_namespace_name);
		if (zend_is_true(lower_case)) {
			phalcon_fast_strtolower(lower_namespace_name, namespace_name);
		} else {
			PHALCON_CPY_WRT_CTOR(lower_namespace_name, namespace_name);
		}

		PHALCON_INIT_VAR(ds_lower_namespace_name);
		phalcon_fast_str_replace(ds_lower_namespace_name, namespace_separator, ds, lower_namespace_name);

		PHALCON_INIT_VAR(layout_namespace);
		PHALCON_CONCAT_SV(layout_namespace, "namespace/", ds_lower_namespace_name);
	}

	/** 
	 * Check if the user has defined a custom layout
	 */
	PHALCON_OBS_VAR(layout);
	phalcon_read_property_this(&layout, this_ptr, SL("_layout"), PH_NOISY TSRMLS_CC);
	if (zend_is_true(layout)) {
		PHALCON_CPY_WRT(layout_name, layout);
	} else if (ds_lower_namespace_name) {
		PHALCON_INIT_VAR(layout_name);
		PHALCON_CONCAT_VSV(layout_name, ds_lower_namespace_name, "/", lower_controller_name);
	} else {
		PHALCON_CPY_WRT(layout_name, lower_controller_name);
	}

	/** 
	 * Load the template engines
	 */
	PHALCON_CALL_METHOD(&engines, this_ptr, "_loadtemplateengines");

	/** 
	 * Check if the user has picked a view diferent than the automatic
	 */
	PHALCON_OBS_VAR(pick_view);
	phalcon_read_property_this(&pick_view, this_ptr, SL("_pickView"), PH_NOISY TSRMLS_CC);
	if (Z_TYPE_P(pick_view) == IS_NULL) {
		PHALCON_INIT_VAR(render_view);

		if (ds_lower_namespace_name) {
			PHALCON_CONCAT_VSVSV(render_view, ds_lower_namespace_name, "/", lower_controller_name, "/", lower_action_name);
		} else {
			PHALCON_CONCAT_VSV(render_view, lower_controller_name, "/", lower_action_name);
		}
	} else {
		/** 
		 * The 'picked' view is an array, where the first element is controller and the
		 * second the action
		 */
		PHALCON_OBS_NVAR(render_view);
		phalcon_array_fetch_long(&render_view, pick_view, 0, PH_NOISY);
		if (phalcon_array_isset_long(pick_view, 1)) {
			PHALCON_OBS_VAR(pick_view_action);
			phalcon_array_fetch_long(&pick_view_action, pick_view, 1, PH_NOISY);
			PHALCON_CPY_WRT(layout_name, pick_view_action);
		}
	}

	/** 
	 * Create a virtual symbol table
	 */
	phalcon_create_symbol_table(TSRMLS_C);

	/** 
	 * Call beforeRender if there is an events manager
	 */
	PHALCON_INIT_NVAR(event_name);
	ZVAL_STRING(event_name, "view:beforeRender", 1);

	PHALCON_CALL_METHOD(&status, this_ptr, "fireeventcancel", event_name);
	if (PHALCON_IS_FALSE(status)) {
		RETURN_MM_FALSE;
	}
	

	/** 
	 * Get the current content in the buffer maybe some output from the controller
	 */
	PHALCON_INIT_NVAR(contents);
	phalcon_ob_get_contents(contents TSRMLS_CC);
	phalcon_update_property_this(this_ptr, SL("_content"), contents TSRMLS_CC);

	PHALCON_INIT_VAR(silence);
	ZVAL_BOOL(silence, 1);

	/** 
	 * Disabled levels allow to avoid an specific level of rendering
	 */
	PHALCON_OBS_VAR(disabled_levels);
	phalcon_read_property_this(&disabled_levels, this_ptr, SL("_disabledLevels"), PH_NOISY TSRMLS_CC);

	/** 
	 * Render level will tell use when to stop
	 */
	PHALCON_OBS_VAR(render_level);
	phalcon_read_property_this(&render_level, this_ptr, SL("_renderLevel"), PH_NOISY TSRMLS_CC);
	if (zend_is_true(render_level)) {
		PHALCON_OBS_VAR(enable_layouts_absolute_path);
		phalcon_read_property_this(&enable_layouts_absolute_path, this_ptr, SL("_enableLayoutsAbsolutePath"), PH_NOISY TSRMLS_CC);

		if (use_model) {
			PHALCON_CALL_METHOD(NULL, view_model, "setview", this_ptr);
			PHALCON_CALL_METHOD(&model_content, view_model, "render");
			phalcon_update_property_this(this_ptr, SL("_content"), model_content TSRMLS_CC);
		}

		if (PHALCON_GE_LONG(render_level, 1)) {
			/** 
			 * Inserts view related to action
			 */
			if (!phalcon_array_isset_long(disabled_levels, 1)) {
				phalcon_update_property_long(this_ptr, SL("_currentRenderLevel"), 1 TSRMLS_CC);
				PHALCON_CALL_METHOD(NULL, this_ptr, "_enginerender", engines, render_view, silence, PHALCON_GLOBAL(z_true));
			}
		}

		/** 
		 * Inserts templates before layout
		 */
		if (PHALCON_GE_LONG(render_level, 2)) {
			if (!phalcon_array_isset_long(disabled_levels, 2)) {
				phalcon_update_property_long(this_ptr, SL("_currentRenderLevel"), 2 TSRMLS_CC);

				PHALCON_OBS_VAR(templates_before);
				phalcon_read_property_this(&templates_before, this_ptr, SL("_templatesBefore"), PH_NOISY TSRMLS_CC);

				/** 
				 * Templates before must be an array
				 */
				if (Z_TYPE_P(templates_before) == IS_ARRAY) { 

					ZVAL_BOOL(silence, 0);

					phalcon_is_iterable(templates_before, &ah0, &hp0, 0, 0);

					while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {

						PHALCON_GET_HVALUE(template_before);

						PHALCON_INIT_NVAR(view_temp_path);
						PHALCON_CONCAT_VV(view_temp_path, layouts_dir, template_before);
						PHALCON_CALL_METHOD(NULL, this_ptr, "_enginerender", engines, view_temp_path, silence, PHALCON_GLOBAL(z_true), enable_layouts_absolute_path);

						zend_hash_move_forward_ex(ah0, &hp0);
					}

					PHALCON_INIT_NVAR(silence);
					ZVAL_BOOL(silence, 1);
				}
			}
		}

		/** 
		 * Inserts controller layout
		 */
		if (PHALCON_GE_LONG(render_level, 3)) {
			if (!phalcon_array_isset_long(disabled_levels, 3)) {
				phalcon_update_property_long(this_ptr, SL("_currentRenderLevel"), 3 TSRMLS_CC);

				PHALCON_INIT_NVAR(view_temp_path);
				PHALCON_CONCAT_VV(view_temp_path, layouts_dir, layout_name);
				PHALCON_CALL_METHOD(NULL, this_ptr, "_enginerender", engines, view_temp_path, silence, PHALCON_GLOBAL(z_true), enable_layouts_absolute_path);
			}
		}

		/** 
		 * Inserts namespace layout
		 */
		if (PHALCON_GE_LONG(render_level, 4) && layout_namespace) {
			if (!phalcon_array_isset_long(disabled_levels, 4)) {
				phalcon_update_property_long(this_ptr, SL("_currentRenderLevel"), 4 TSRMLS_CC);

				PHALCON_INIT_NVAR(view_temp_path);
				PHALCON_CONCAT_VV(view_temp_path, layouts_dir, layout_namespace);
				PHALCON_CALL_METHOD(NULL, this_ptr, "_enginerender", engines, view_temp_path, silence, PHALCON_GLOBAL(z_true), enable_layouts_absolute_path);
			}
		}

		/** 
		 * Inserts templates after layout
		 */
		if (PHALCON_GE_LONG(render_level, 5)) {
			if (!phalcon_array_isset_long(disabled_levels, 5)) {
				phalcon_update_property_long(this_ptr, SL("_currentRenderLevel"), 5 TSRMLS_CC);

				/** 
				 * Templates after must be an array
				 */
				PHALCON_OBS_VAR(templates_after);
				phalcon_read_property_this(&templates_after, this_ptr, SL("_templatesAfter"), PH_NOISY TSRMLS_CC);
				if (Z_TYPE_P(templates_after) == IS_ARRAY) { 

					PHALCON_INIT_NVAR(silence);
					ZVAL_BOOL(silence, 0);

					phalcon_is_iterable(templates_after, &ah1, &hp1, 0, 0);

					while (zend_hash_get_current_data_ex(ah1, (void**) &hd, &hp1) == SUCCESS) {

						PHALCON_GET_HVALUE(template_after);

						PHALCON_INIT_NVAR(view_temp_path);
						PHALCON_CONCAT_VV(view_temp_path, layouts_dir, template_after);
						PHALCON_CALL_METHOD(NULL, this_ptr, "_enginerender", engines, view_temp_path, silence, PHALCON_GLOBAL(z_true), enable_layouts_absolute_path);

						zend_hash_move_forward_ex(ah1, &hp1);
					}

					PHALCON_INIT_NVAR(silence);
					ZVAL_BOOL(silence, 1);
				}
			}
		}

		/** 
		 * Inserts main view
		 */
		if (PHALCON_GE_LONG(render_level, 6)) {
			if (!phalcon_array_isset_long(disabled_levels, 6)) {
				phalcon_update_property_long(this_ptr, SL("_currentRenderLevel"), 6 TSRMLS_CC);

				PHALCON_OBS_VAR(main_view);
				phalcon_read_property_this(&main_view, this_ptr, SL("_mainView"), PH_NOISY TSRMLS_CC);
				PHALCON_CALL_METHOD(NULL, this_ptr, "_enginerender", engines, main_view, silence, PHALCON_GLOBAL(z_true));
			}
		}

		phalcon_update_property_this(this_ptr, SL("_currentRenderLevel"), PHALCON_GLOBAL(z_zero) TSRMLS_CC);
	}

	/** 
	 * Call afterRender event
	 */
	PHALCON_INIT_NVAR(event_name);
	ZVAL_STRING(event_name, "view:afterRender", 1);
	PHALCON_CALL_METHOD(NULL, this_ptr, "fireevent", event_name);

	RETURN_THIS();
}

/**
 * Choose a different view to render instead of last-controller/last-action
 *
 * <code>
 * class ProductsController extends Phalcon\Mvc\Controller
 * {
 *
 *    public function saveAction()
 *    {
 *
 *         //Do some save stuff...
 *
 *         //Then show the list view
 *         $this->view->pick("products/list");
 *    }
 * }
 * </code>
 *
 * @param string|array $renderView
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, pick){

	zval *render_view, *pick_view = NULL, *layout = NULL, *parts;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &render_view);

	if (Z_TYPE_P(render_view) == IS_ARRAY) { 
		PHALCON_CPY_WRT(pick_view, render_view);
	} else {
		PHALCON_INIT_VAR(layout);
		if (phalcon_memnstr_str(render_view, SL("/"))) {
			PHALCON_INIT_VAR(parts);
			phalcon_fast_explode_str(parts, SL("/"), render_view);

			PHALCON_OBS_NVAR(layout);
			phalcon_array_fetch_long(&layout, parts, 0, PH_NOISY);
		}

		PHALCON_INIT_NVAR(pick_view);
		array_init_size(pick_view, 2);
		phalcon_array_append(&pick_view, render_view, PH_COPY);
		if (Z_TYPE_P(layout) != IS_NULL) {
			phalcon_array_append(&pick_view, layout, PH_COPY);
		}
	}
	phalcon_update_property_this(this_ptr, SL("_pickView"), pick_view TSRMLS_CC);

	RETURN_THIS();
}

/**
 * Renders a partial view
 *
 * <code>
 * 	//Show a partial inside another view
 * 	$this->partial('shared/footer');
 * </code>
 *
 * <code>
 * 	//Show a partial inside another view with parameters
 * 	$this->partial('shared/footer', array('content' => $html));
 * </code>
 *
 * @param string $partialPath
 * @param array $params
 * @param boolean $autorender
 */
PHP_METHOD(Phalcon_Mvc_View, partial){

	zval *partial_path, *params = NULL, *autorender = NULL, *view_params, *new_params = NULL;
	zval *partials_dir, *enable_partials_absolute_path, *real_path, *engines = NULL;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 2, &partial_path, &params, &autorender);

	if (!params) {
		params = PHALCON_GLOBAL(z_null);
	}

	if (!autorender) {
		autorender = PHALCON_GLOBAL(z_true);
	}

	/** 
	 * If the developer pass an array of variables we create a new virtual symbol table
	 */
	if (Z_TYPE_P(params) == IS_ARRAY) { 

		PHALCON_OBS_VAR(view_params);
		phalcon_read_property_this(&view_params, this_ptr, SL("_viewParams"), PH_NOISY TSRMLS_CC);

		/** 
		 * Merge or assign the new params as parameters
		 */
		if (Z_TYPE_P(view_params) == IS_ARRAY) { 
			PHALCON_INIT_VAR(new_params);
			phalcon_fast_array_merge(new_params, &view_params, &params TSRMLS_CC);
		} else {
			PHALCON_CPY_WRT(new_params, params);
		}

		/** 
		 * Update the parameters with the new ones
		 */
		phalcon_update_property_this(this_ptr, SL("_viewParams"), new_params TSRMLS_CC);

		/** 
		 * Create a virtual symbol table
		 */
		phalcon_create_symbol_table(TSRMLS_C);

	}

	PHALCON_OBS_VAR(partials_dir);
	phalcon_read_property_this(&partials_dir, this_ptr, SL("_partialsDir"), PH_NOISY TSRMLS_CC);

	PHALCON_OBS_VAR(enable_partials_absolute_path);
	phalcon_read_property_this(&enable_partials_absolute_path, this_ptr, SL("_enablePartialsAbsolutePath"), PH_NOISY TSRMLS_CC);

	/** 
	 * Partials are looked up under the partials directory
	 */
	PHALCON_INIT_VAR(real_path);
	PHALCON_CONCAT_VV(real_path, partials_dir, partial_path);

	/** 
	 * We need to check if the engines are loaded first, this method could be called
	 * outside of 'render'
	 */
	PHALCON_CALL_METHOD(&engines, this_ptr, "_loadtemplateengines");

	/** 
	 * Call engine render, this checks in every registered engine for the partial
	 */
	PHALCON_CALL_METHOD(NULL, this_ptr, "_enginerender", engines, real_path, PHALCON_GLOBAL(z_false), PHALCON_GLOBAL(z_false), enable_partials_absolute_path);

	/** 
	 * Now we need to restore the original view parameters
	 */
	if (Z_TYPE_P(params) == IS_ARRAY) { 
		/** 
		 * Restore the original view params
		 */
		phalcon_update_property_this(this_ptr, SL("_viewParams"), view_params TSRMLS_CC);
	}

	if (!PHALCON_IS_TRUE(autorender)) {
		phalcon_ob_get_contents(return_value TSRMLS_CC);
		phalcon_ob_clean(TSRMLS_C);
	}

	PHALCON_MM_RESTORE();
}

/**
 * Perform the automatic rendering returning the output as a string
 *
 * <code>
 * 	$template = $this->view->getRender('products', 'show', array('products' => $products));
 * </code>
 *
 * @param string $controllerName
 * @param string $actionName
 * @param array $params
 * @param mixed $configCallback
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getRender){

	zval *controller_name, *action_name, *params = NULL;
	zval *config_callback = NULL, *view, *status;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 2, 2, &controller_name, &action_name, &params, &config_callback);

	if (!params) {
		PHALCON_INIT_VAR(params);
	} else {
		PHALCON_SEPARATE_PARAM(params);
	}

	if (!config_callback) {
		config_callback = PHALCON_GLOBAL(z_null);
	}

	/** 
	 * We must to clone the current view to keep the old state
	 */
	PHALCON_INIT_VAR(view);
	if (phalcon_clone(view, this_ptr TSRMLS_CC) == FAILURE) {
		RETURN_MM();
	}

	/** 
	 * The component must be reset to its defaults
	 */
	PHALCON_CALL_METHOD(NULL, view, "reset");

	/** 
	 * Set the render variables
	 */
	if (Z_TYPE_P(params) == IS_ARRAY) { 
		PHALCON_CALL_METHOD(NULL, view, "setvars", params);
	}

	/** 
	 * Perform extra configurations over the cloned object
	 */
	if (Z_TYPE_P(config_callback) == IS_OBJECT) {
		PHALCON_INIT_NVAR(params);
		array_init_size(params, 1);
		phalcon_array_append(&params, view, PH_COPY);

		PHALCON_INIT_VAR(status);/**/
		PHALCON_CALL_USER_FUNC_ARRAY(status, config_callback, params);
	}

	/** 
	 * Start the output buffering
	 */
	PHALCON_CALL_METHOD(NULL, view, "start");

	/** 
	 * Perform the render passing only the controller and action
	 */
	PHALCON_CALL_METHOD(NULL, view, "render", controller_name, action_name);

	/** 
	 * Stop the output buffering
	 */
	phalcon_ob_end_clean(TSRMLS_C);

	/** 
	 * Get the processed content
	 */
	PHALCON_RETURN_CALL_METHOD(view, "getcontent");

	PHALCON_MM_RESTORE();
}

/**
 * Finishes the render process by stopping the output buffering
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, finish){

	phalcon_ob_end_clean(TSRMLS_C);
	RETURN_THISW();
}

/**
 * Create a Phalcon\Cache based on the internal cache options
 *
 * @return Phalcon\Cache\BackendInterface
 */
PHP_METHOD(Phalcon_Mvc_View, _createCache){

	zval *dependency_injector = NULL, *cache_service = NULL;
	zval *view_options, *cache_options, *view_cache = NULL;

	PHALCON_MM_GROW();

	PHALCON_CALL_METHOD(&dependency_injector, this_ptr, "getdi");

	PHALCON_INIT_VAR(cache_service);
	ZVAL_STRING(cache_service, "viewCache", 1);

	PHALCON_OBS_VAR(view_options);
	phalcon_read_property_this(&view_options, this_ptr, SL("_options"), PH_NOISY TSRMLS_CC);
	if (Z_TYPE_P(view_options) == IS_ARRAY) { 
		if (phalcon_array_isset_string(view_options, SS("cache"))) {

			PHALCON_OBS_VAR(cache_options);
			phalcon_array_fetch_string(&cache_options, view_options, SL("cache"), PH_NOISY);
			if (Z_TYPE_P(cache_options) == IS_ARRAY) { 
				if (phalcon_array_isset_string(cache_options, SS("service"))) {
					PHALCON_OBS_NVAR(cache_service);
					phalcon_array_fetch_string(&cache_service, cache_options, SL("service"), PH_NOISY);
				}
			}
		}
	}

	/** 
	 * The injected service must be an object
	 */
	PHALCON_CALL_METHOD(&view_cache, dependency_injector, "getshared", cache_service);
	if (Z_TYPE_P(view_cache) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_view_exception_ce, "The injected caching service is invalid");
		return;
	}

	PHALCON_VERIFY_INTERFACE(view_cache, phalcon_cache_backendinterface_ce);
	RETURN_CCTOR(view_cache);
}

/**
 * Check if the component is currently caching the output content
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_View, isCaching){

	zval *z_zero, *cache_level;

	z_zero = PHALCON_GLOBAL(z_zero);

	cache_level = phalcon_fetch_nproperty_this(this_ptr, SL("_cacheLevel"), PH_NOISY TSRMLS_CC);
	is_smaller_function(return_value, z_zero, cache_level TSRMLS_CC);
}

/**
 * Returns the cache instance used to cache
 *
 * @return Phalcon\Cache\BackendInterface
 */
PHP_METHOD(Phalcon_Mvc_View, getCache){

	zval *cache = NULL;

	PHALCON_MM_GROW();

	PHALCON_OBS_VAR(cache);
	phalcon_read_property_this(&cache, this_ptr, SL("_cache"), PH_NOISY TSRMLS_CC);
	if (zend_is_true(cache)) {
		if (Z_TYPE_P(cache) != IS_OBJECT) {
			PHALCON_CALL_METHOD(&cache, this_ptr, "_createcache");
			phalcon_update_property_this(this_ptr, SL("_cache"), cache TSRMLS_CC);
		}
	} else {
		PHALCON_CALL_METHOD(&cache, this_ptr, "_createcache");
		phalcon_update_property_this(this_ptr, SL("_cache"), cache TSRMLS_CC);
	}

	RETURN_CCTOR(cache);
}

/**
 * Cache the actual view render to certain level
 *
 *<code>
 *  $this->view->cache(array('key' => 'my-key', 'lifetime' => 86400));
 *</code>
 *
 * @param boolean|array $options
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, cache){

	zval *options = NULL, *view_options = NULL, *cache_options = NULL;
	zval *value = NULL, *key = NULL, *cache_level, *cache_mode;
	HashTable *ah0;
	HashPosition hp0;
	zval **hd;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 0, 1, &options);

	if (!options) {
		options = PHALCON_GLOBAL(z_true);
	}

	if (Z_TYPE_P(options) == IS_ARRAY) { 

		PHALCON_OBS_VAR(view_options);
		phalcon_read_property_this(&view_options, this_ptr, SL("_options"), PH_NOISY TSRMLS_CC);
		if (Z_TYPE_P(view_options) != IS_ARRAY) { 
			PHALCON_INIT_NVAR(view_options);
			array_init(view_options);
		}

		/** 
		 * Get the default cache options
		 */
		if (phalcon_array_isset_string(view_options, SS("cache"))) {
			PHALCON_OBS_VAR(cache_options);
			phalcon_array_fetch_string(&cache_options, view_options, SL("cache"), PH_NOISY);
		} else {
			PHALCON_INIT_NVAR(cache_options);
			array_init(cache_options);
		}

		phalcon_is_iterable(options, &ah0, &hp0, 0, 0);

		while (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) == SUCCESS) {

			PHALCON_GET_HKEY(key, ah0, hp0);
			PHALCON_GET_HVALUE(value);

			phalcon_array_update_zval(&cache_options, key, value, PH_COPY);

			zend_hash_move_forward_ex(ah0, &hp0);
		}

		/** 
		 * Check if the user has defined a default cache level or use 5 as default
		 */
		if (phalcon_array_isset_string(cache_options, SS("level"))) {
			PHALCON_OBS_VAR(cache_level);
			phalcon_array_fetch_string(&cache_level, cache_options, SL("level"), PH_NOISY);
			phalcon_update_property_this(this_ptr, SL("_cacheLevel"), cache_level TSRMLS_CC);
		} else {
			phalcon_update_property_long(this_ptr, SL("_cacheLevel"), 5 TSRMLS_CC);
		}

		if (phalcon_array_isset_string(cache_options, SS("mode"))) {
			PHALCON_OBS_VAR(cache_mode);
			phalcon_array_fetch_string(&cache_mode, cache_options, SL("mode"), PH_NOISY);
			phalcon_update_property_this(this_ptr, SL("_cacheMode"), cache_mode TSRMLS_CC);
		} else {
			phalcon_update_property_bool(this_ptr, SL("_cacheMode"), 0 TSRMLS_CC);
		}

		phalcon_array_update_string(&view_options, SL("cache"), cache_options, PH_COPY);
		phalcon_update_property_this(this_ptr, SL("_options"), view_options TSRMLS_CC);
	} else {
		/** 
		 * If 'options' isn't an array we enable the cache with the default options
		 */
		if (zend_is_true(options)) {
			phalcon_update_property_long(this_ptr, SL("_cacheLevel"), 5 TSRMLS_CC);
		} else {
			phalcon_update_property_long(this_ptr, SL("_cacheLevel"), 0 TSRMLS_CC);
		}

		phalcon_update_property_bool(this_ptr, SL("_cacheMode"), 0 TSRMLS_CC);
	}

	RETURN_THIS();
}

/**
 * Externally sets the view content
 *
 *<code>
 *	$this->view->setContent("<h1>hello</h1>");
 *</code>
 *
 * @param string $content
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setContent){

	zval *content;

	phalcon_fetch_params(0, 1, 0, &content);

	if (Z_TYPE_P(content) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_view_exception_ce, "Content must be a string");
		return;
	}
	phalcon_update_property_this(this_ptr, SL("_content"), content TSRMLS_CC);

	RETURN_THISW();
}

/**
 * Returns cached output from another view stage
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getContent){


	RETURN_MEMBER(this_ptr, "_content");
}

/**
 * Returns the path of the view that is currently rendered
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View, getActiveRenderPath){


	RETURN_MEMBER(this_ptr, "_activeRenderPath");
}

/**
 * Disables the auto-rendering process
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, disable){


	phalcon_update_property_bool(this_ptr, SL("_disabled"), 1 TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Enables the auto-rendering process
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, enable){


	phalcon_update_property_bool(this_ptr, SL("_disabled"), 0 TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Whether automatic rendering is enabled
 *
 * @return bool
 */
PHP_METHOD(Phalcon_Mvc_View, isDisabled){

	RETURN_MEMBER(this_ptr, "_disabled");
}


/**
 * Resets the view component to its factory default values
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, reset){

	zval *z_null  = PHALCON_GLOBAL(z_null);
	zval *z_false = PHALCON_GLOBAL(z_false);
	zval *z_zero  = PHALCON_GLOBAL(z_zero);

	phalcon_update_property_this(this_ptr, SL("_disabled"), z_false TSRMLS_CC);
	phalcon_update_property_this(this_ptr, SL("_engines"), z_false TSRMLS_CC);
	phalcon_update_property_this(this_ptr, SL("_cache"), z_null TSRMLS_CC);
	phalcon_update_property_long(this_ptr, SL("_renderLevel"), 5 TSRMLS_CC);
	phalcon_update_property_zval(this_ptr, SL("_cacheLevel"), z_zero TSRMLS_CC);
	phalcon_update_property_this(this_ptr, SL("_content"), z_null TSRMLS_CC);
	phalcon_update_property_this(this_ptr, SL("_templatesBefore"), z_null TSRMLS_CC);
	phalcon_update_property_this(this_ptr, SL("_templatesAfter"), z_null TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Magic method to pass variables to the views
 *
 *<code>
 *	$this->view->products = $products;
 *</code>
 *
 * @param string $key
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Mvc_View, __set){

	zval *key, *value;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	phalcon_update_property_array(this_ptr, SL("_viewParams"), key, value TSRMLS_CC);

}

/**
 * Magic method to retrieve a variable passed to the view
 *
 *<code>
 *	echo $this->view->products;
 *</code>
 *
 * @param string $key
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_View, __get){

	zval *key, *params, *value;

	phalcon_fetch_params(0, 1, 0, &key);

	params = phalcon_fetch_nproperty_this(this_ptr, SL("_viewParams"), PH_NOISY TSRMLS_CC);
	if (phalcon_array_isset_fetch(&value, params, key)) {
		RETURN_ZVAL(value, 1, 0);
	}

	RETURN_NULL();
}

/**
 * Magic method to inaccessible a variable passed to the view
 *
 *<code>
 *	isset($this->view->products)
 *</code>
 *
 * @param string $key
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_View, __isset){

	zval *key, *params;

	phalcon_fetch_params(0, 1, 0, &key);

	params = phalcon_fetch_nproperty_this(this_ptr, SL("_viewParams"), PH_NOISY TSRMLS_CC);
	if (phalcon_array_isset(params, key)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Enables the auto-rendering process
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, enableNamespaceView){

	phalcon_update_property_bool(this_ptr, SL("_enableNamespaceView"), 1 TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Whether automatic rendering is enabled
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, disableNamespaceView){

	phalcon_update_property_bool(this_ptr, SL("_enableNamespaceView"), 0 TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Enables to lower case view path
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, enableLowerCase){

	phalcon_update_property_bool(this_ptr, SL("_lowerCase"), 1 TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Whether to lower case view path
 *
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, disableLowerCase){

	phalcon_update_property_bool(this_ptr, SL("_lowerCase"), 0 TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Adds a converter
 *
 * @param string $name
 * @param callable $converter
 * @return Phalcon\Mvc\View
 */
PHP_METHOD(Phalcon_Mvc_View, setConverter){

	zval **name, **converter;

	phalcon_fetch_params_ex(2, 0, &name, &converter);	

	if (!phalcon_is_callable(*converter TSRMLS_CC)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_view_exception_ce, "The paramter `converter` is not callable");
		return;
	}

	phalcon_update_property_array(this_ptr, SL("_converters"), *name, *converter TSRMLS_CC);
	RETURN_THISW();
}

/**
 * Returns the router converter
 *
 * @return callable|null
 */
PHP_METHOD(Phalcon_Mvc_View, getConverter) {

	zval *name, *converters, *converter;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &name);

	PHALCON_OBS_VAR(converters);
	phalcon_read_property_this(&converters, this_ptr, SL("_converters"), PH_NOISY TSRMLS_CC);

	if (phalcon_array_isset(converters, name)) {
		PHALCON_OBS_VAR(converter);
		phalcon_array_fetch(&converter, converters, name, PH_NOISY);

		RETURN_CTOR(converter);
	}

	RETURN_MM_NULL();
}
