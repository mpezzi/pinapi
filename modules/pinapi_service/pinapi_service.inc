<?php 

/**
 * @file
 * Link general PinAPI functionality to services module.
 */


/**
 * Redeems a code.
 */
function pinapi_service_code_redeem($params) {
  if ( $account = user_load($params['redeem']['uid']) ) {
    if ( $prize = pinapi_redeem($params['redeem']['code'], $params['redeem']['pool_id'], $account) ) {
      return $prize;
    }
    else {
      return services_error(t('Code is not valid or has already been redeemed.'), 401);
    }
  }
  else {
    return services_error(t("The user doesn't exist."), 401);
  }
}
