<?php
// $Id: user_trend.pages.inc $

drupal_add_css(drupal_get_path('module', 'user_trend') . '/user_trend_style.css');
define('UT_YEAR_START', 2014);
define('UT_YEAR_END', 2020);

/**
 * Page callback to show Yearly trend of user login
 */
function yearly_trend_report_page() {
  $page = "";
  $page .= "<div id = 'user_trend_wrapper'>";
  $page .= drupal_render(drupal_get_form("yearly_trend_form")); 
  $page .= yearly_trend_report(); 
  $page .= "</div>";
  return $page;

}

function yearly_trend_form($form) {
  // Find out the current year
  $var_from_year  = arg(4) == '' ? date('Y') : arg(4);
  $var_to_year    = arg(5) == '' ? date('Y') : arg(5);

  $form['sel_from_year'] = array(
    '#title' => t('Select From Year'),
    '#type' => 'select',
    '#default_value' => $var_from_year,
    '#options' => _get_years_options(),
  );
  $form['sel_to_year'] = array(
    '#title' => t('Select To Year'),
    '#type' => 'select',
    '#default_value' => $var_to_year,
    '#options' => _get_years_options(),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Go'),
  );

  return $form;
}

/**
 * Implementation of hook_validate().
 */
function yearly_trend_form_validate($form, &$form_state) {
  $from_year = $form_state['values']['sel_from_year'];
  $to_year = $form_state['values']['sel_to_year'];

  if ($from_year > $to_year) {
    form_set_error('sel_from_year', t("'From Year' can't be greater than 'To Year'"));
  }
}

/**
 * Implementation of hook_submit().
 */
function yearly_trend_form_submit($form, &$form_state) {
  $from_year = $form_state['values']['sel_from_year'];
  $to_year = $form_state['values']['sel_to_year'];
  drupal_goto("admin/reports/user_trend/yearly/" . $from_year . "/" . $to_year);
}

function yearly_trend_report() {
  $var_from_year  = arg(4) == '' ? date('Y') : arg(4);
  $var_to_year    = arg(5) == '' ? date('Y') : arg(5);

  $header[] = array('data' => t('Year'));
  $header[] = array('data' => t('Total User Logins'));

  $query = db_select('user_trend', 'ut');
  $query->addExpression("FROM_UNIXTIME(login_time, '%Y')", 'login_date');
  $query->addExpression("COUNT(*)", 'value_count');
  $query->where("FROM_UNIXTIME(login_time,'%Y') >= :from_year 
    AND FROM_UNIXTIME(login_time,'%Y') <= :to_year",
    array(':from_year' => $var_from_year, 'to_year' => $var_to_year))
    ->groupBy("FROM_UNIXTIME(login_time,'%Y')")
    ->orderBy('login_time');
  $result = $query->extend('PagerDefault')->limit(5)->execute();

  $rows = _get_rows_from_result($result);
  $output = theme('table', array('header' => $header, 'rows' => $rows));
  return $output . theme('pager');
}

/**
 * @param $result
 * @return array
 */
function _get_rows_from_result($result) {
  $rows = array();
  $num_rows = FALSE;
  foreach ($result as $row) {
    $rows[] = array(
      $row->login_date,
      $row->value_count,
    );
    $num_rows = TRUE;
  }
  if (!$num_rows) {
    $rows[][] = array(
      'data' => t('No user logged in during this period.'),
      'class' => 'report_empty_cell',
      'colspan' => 2,
    );
  }
  return $rows;
}


/*****************************************************************************/
/**
 * Page callback to show Quarterly trend of user login
 */
function quarterly_trend_report_page() {
  $page = "";
  $page .= "<div id = 'user_trend_wrapper'>";
  $page .= drupal_render(drupal_get_form("quarterly_trend_form")); 
  $page .= quarterly_trend_report(); 
  $page .= "</div>";
  return $page;
}

function quarterly_trend_form($form) {
  // Find out the current year
  $var_year     = arg(4) == '' ? date('Y') : arg(4);
  $var_quarter  = arg(5) == '' ? '1' : arg(5);

  $form['sel_year'] = array(
    '#title' => t('Select Year'),
    '#type' => 'select',
    '#default_value' => $var_year,
    '#options' => _get_years_options(),
  );
  $form['sel_quarter'] = array(
    '#title' => t('Select Quarter'),
    '#type' => 'select',
    '#default_value' => $var_quarter,
    '#options' => array(
      '1' => t('Quarter 1'),
      '2' => t('Quarter 2'),
      '3' => t('Quarter 3'),
      '4' => t('Quarter 4'),
    ),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Go'),
  );

  return $form;
}

/**
 * Implementation of hook_submit().
 */
function quarterly_trend_form_submit($form, &$form_state) {
  $year     = $form_state['values']['sel_year'];
  $quarter  = $form_state['values']['sel_quarter'];
  drupal_goto('admin/reports/user_trend/quarterly/' . $year . '/' . $quarter);
}

function quarterly_trend_report() {
  $var_year     = arg(4) == '' ? date('Y') : arg(4);
  $var_quarter  = arg(5) == '' ? 1 : arg(5);

  $quarterArray = _get_quarter_period($var_quarter, $var_year);
  $varFromTimestamp = $quarterArray['start'];
  $varToTimestamp = $quarterArray['end'];

  $header[] = array('data' => t('Month'));
  $header[] = array('data' => t('Total User Logins'));

  $query = db_select('user_trend', 'ut');
  $query->addExpression("FROM_UNIXTIME(login_time, '%M, %Y')", 'login_date');
  $query->addExpression("COUNT(*)", 'value_count');
  $query->where("login_time >= :from AND login_time <= :to",
    array(':from' => $varFromTimestamp, 'to' => $varToTimestamp))
    ->groupBy("FROM_UNIXTIME(login_time,'%m %Y')")
    ->orderBy('login_time');
  $result = $query
    ->extend('PagerDefault')
    ->limit(5)
    ->execute();

  $rows = _get_rows_from_result($result);
  // Make the table...
  $output = theme('table', array('header' => $header, 'rows' => $rows));
  return $output . theme('pager');
}


/*****************************************************************************/
/**
 * Page callback to show Monthly trend of user login
 */
function monthly_trend_report_page() {
  $page = "";
  $page .= "<div id = 'user_trend_wrapper'>";
  $page .= drupal_render(drupal_get_form("monthly_trend_form")); 
  $page .= monthly_trend_report(); 
  $page .= "</div>";
  return $page;
}

function monthly_trend_form($form) {
  // Find out the current year & month
  $var_from_mon   = arg(4) == '' ? date('m') : arg(4);
  $var_from_year  = arg(5) == '' ? date('Y') : arg(5);
  $var_to_mon     = arg(6) == '' ? date('m') : arg(6);
  $var_to_year    = arg(7) == '' ? date('Y') : arg(7);

  $form['sel_from_month'] = array(
    '#type' => 'select',
    '#title' => t('Select From Month & Year'),
    '#default_value' => $var_from_mon,
    '#options' => _get_months_options(),
  );
  $form['sel_from_year'] = array(
    '#type' => 'select',
    '#default_value' => $var_from_year,
    '#options' => _get_years_options(),
  );
  $form['sel_to_month'] = array(
    '#type' => 'select',
    '#title' => t('Select To Month & Year'),
    '#default_value' => $var_to_mon,
    '#options' => _get_months_options(),
  );
  $form['sel_to_year'] = array(
    '#type' => 'select',
    '#default_value' => $var_to_year,
    '#options' => _get_years_options(),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Go'),
  );

  return $form;
}

/**
 * Implementation of hook_validate().
 */
function monthly_trend_form_validate($form, &$form_state) {
  $from_mon   = $form_state['values']['sel_from_month'];
  $from_year  = $form_state['values']['sel_from_year'];
  $to_mon     = $form_state['values']['sel_to_month'];
  $to_year    = $form_state['values']['sel_to_year'];

  if ($from_year > $to_year) {
    form_set_error('sel_from_year', t("'From Year' can't be greater than 'To Year'"));
  }
  elseif ($from_year == $to_year && $from_mon > $to_mon) {
    form_set_error('sel_from_month', t("'From Month' can't be greater than 'To Month'"));
  }
}

/**
 * Implementation of hook_submit().
 */
function monthly_trend_form_submit($form, &$form_state) {
  $from_mon = $form_state['values']['sel_from_month'];
  $from_year = $form_state['values']['sel_from_year'];
  $to_mon = $form_state['values']['sel_to_month'];
  $to_year = $form_state['values']['sel_to_year'];
  drupal_goto('admin/reports/user_trend/monthly/'
    . $from_mon . '/' . $from_year . '/' . $to_mon . '/' . $to_year);
}

/**
 * Generates report: builds and executes query, returns HTML output
 * @return string
 */
function monthly_trend_report() {
  $var_from_mon   = arg(4) == '' ? date('m') : arg(4);
  $var_from_year  = arg(5) == '' ? date('Y') : arg(5);
  $var_to_mon     = arg(6) == '' ? date('m') : arg(6);
  $var_to_year    = arg(7) == '' ? date('Y') : arg(7);
  $varFromTimestamp = strtotime($var_from_year . '-' . $var_from_mon . '-'
    . _firstOfMonth($var_from_mon, $var_from_year) . '  00:00:00');
  $varToTimestamp = strtotime($var_to_year . '-' . $var_to_mon . '-'
    . _lastOfMonth($var_to_mon, $var_to_year) . '  23:59:00');

  $header[] = array('data' => t('Month'));
  $header[] = array('data' => t('Total User Logins'));

  $query = db_select('user_trend', 'ut');
  $query->addExpression("FROM_UNIXTIME(login_time, '%M, %Y')", 'login_date');
  $query->addExpression("COUNT(*)", 'value_count');
  $query->where("login_time >= :from AND login_time <= :to",
    array(':from' => $varFromTimestamp, 'to' => $varToTimestamp))
    ->groupBy("FROM_UNIXTIME(login_time,'%m %Y')")
    ->orderBy('login_time');
  $result = $query->extend('PagerDefault')->limit(5)->execute();

  $rows = _get_rows_from_result($result);
  $output = theme('table', array('header' => $header, 'rows' => $rows));
  return $output . theme('pager');
}

/*****************************************************************************/
/**
 * Page callback to show Monthly trend of user login
 */
function weekly_trend_report_page() {
  $page = "";
  $page .= "<div id = 'user_trend_wrapper'>";
  $page .= drupal_render(drupal_get_form("weekly_trend_form")); 
  $page .= weekly_trend_report(); 
  $page .= "</div>";
  return $page;
}

function weekly_trend_form($form) {
  // Find out the current year & month
  $var_from_mon   = arg(4) == '' ? date('m') : arg(4);
  $var_from_year  = arg(5) == '' ? date('Y') : arg(5);
  $var_to_mon     = arg(6) == '' ? date('m') : arg(6);
  $var_to_year    = arg(7) == '' ? date('Y') : arg(7);
  $var_from_week  = arg(8) == '' ? 1 : arg(8);
  $var_to_week    = arg(9) == '' ? 1 : arg(9);

  $form['sel_from_week'] = array(
    '#type' => 'select',
    '#title' => t('Select From Week, Month and Year'),
    '#default_value' => $var_from_week,
    '#options' => _get_weeks_options(),
  );
  $form['sel_from_month'] = array(
    '#type' => 'select',
    '#default_value' => $var_from_mon,
    '#options' => _get_months_options(),
  );
  $form['sel_from_year'] = array(
    '#type' => 'select',
    '#default_value' => $var_from_year,
    '#options' => _get_years_options(),
  );

  $form['sel_to_week'] = array(
    '#type' => 'select',
    '#title' => t('Select To Week, Month and Year'),
    '#default_value' => $var_to_week,
    '#options' => _get_weeks_options(),
  );
  $form['sel_to_month'] = array(
    '#type' => 'select',
    '#default_value' => $var_to_mon,
    '#options' => _get_months_options(),
  );
  $form['sel_to_year'] = array(
    '#type' => 'select',
    '#default_value' => $var_to_year,
    '#options' => _get_years_options(),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Go'),
  );

  return $form;
}

/**
 * Implementation of hook_validate().
 */
function weekly_trend_form_validate($form, &$form_state) {
  $from_week  = $form_state['values']['sel_from_week'];
  $from_mon   = $form_state['values']['sel_from_month'];
  $from_year  = $form_state['values']['sel_from_year'];
  $to_week    = $form_state['values']['sel_to_week'];
  $to_mon     = $form_state['values']['sel_to_month'];
  $to_year    = $form_state['values']['sel_to_year'];

  if ($from_year > $to_year) {
    form_set_error('sel_from_year', t("'From Year' can't be greater than 'To Year'"));
  }
  if ($from_year == $to_year && $from_mon > $to_mon) {
    form_set_error('sel_from_month', t("'From Month' can't be greater than 'To Month'"));
  }
  if ($from_year == $to_year && $from_mon == $to_mon && $from_week > $to_week) {
    form_set_error('sel_from_week', t("'From Week' can't be greater than 'To Week'"));
  }
}

/**
 * Implementation of hook_submit().
 */
function weekly_trend_form_submit($form, &$form_state) {
  $from_mon = $form_state['values']['sel_from_month'];
  $from_year = $form_state['values']['sel_from_year'];
  $to_mon = $form_state['values']['sel_to_month'];
  $to_year = $form_state['values']['sel_to_year'];
  $from_week = $form_state['values']['sel_from_week'];
  $to_week = $form_state['values']['sel_to_week'];
  drupal_goto('admin/reports/user_trend/weekly/'
    . $from_mon . '/' . $from_year . '/' . $to_mon . '/' . $to_year
    . '/' . $from_week . '/' . $to_week);
}

/**
 * Generates report: builds and executes query, returns HTML output
 * @return string
 */
function weekly_trend_report() {
  $var_from_mon   = arg(4) == '' ? date('m') : arg(4);
  $var_from_year  = arg(5) == '' ? date('Y') : arg(5);
  $var_to_mon     = arg(6) == '' ? date('m') : arg(6);
  $var_to_year    = arg(7) == '' ? date('Y') : arg(7);
  $from_week      = arg(8) == '' ? '01' : arg(8);
  $to_week        = arg(9) == '' ? '01' : arg(9);

  $from_year_month = $var_from_year . '-' . $var_from_mon . '-';
  $from_date = $from_year_month . _firstOfWeek($from_week, $var_from_mon, $var_from_year);
  $varFromTimestamp = strtotime($from_date . '  00:00:00');
  $to_year_month = $var_to_year . '-' . $var_to_mon . '-';
  $to_date = $to_year_month . _lastOfWeek($to_week, $var_to_mon, $var_to_year);
  $varToTimestamp = strtotime($to_date . '  23:59:00');

  $header[] = array('data' => t('Day'));
  $header[] = array('data' => t('Total User Logins'));

  $query = db_select('user_trend', 'ut');
  $query->addExpression("FROM_UNIXTIME(login_time, '%d, %M, %Y')", 'login_date');
  $query->addExpression("COUNT(*)", 'value_count');
  $query->where("login_time >= :from AND login_time <= :to",
    array(':from' => $varFromTimestamp, ':to' => $varToTimestamp))
    ->groupBy("FROM_UNIXTIME(login_time,'%d, %M, %Y')")
    ->orderBy('login_time');
  $result = $query->extend('PagerDefault')->limit(5)->execute();

  $rows = _get_rows_from_result($result);
  $output = theme('table', array('header' => $header, 'rows' => $rows));
  return $output . theme('pager');
}

/*****************************************************************************/
/**
 * Page callback to show Monthly trend of user login
 */
function daily_trend_report_page() {
  $page = "";
  $page .= "<div id = 'user_trend_wrapper'>";
  $page .= drupal_render(drupal_get_form("daily_trend_form")); 
  $page .= daily_trend_report(); 
  $page .= "</div>";
  return $page;
}

function daily_trend_form($form) {
  // Find out the current year & month
  $var_from_mon   = arg(4) == '' ? date('m') : arg(4);
  $var_from_year  = arg(5) == '' ? date('Y') : arg(5);
  $var_to_mon     = arg(6) == '' ? date('m') : arg(6);
  $var_to_year    = arg(7) == '' ? date('Y') : arg(7);
  $var_from_day   = arg(8) == '' ? 1 : arg(8);
  $var_to_day     = arg(9) == '' ? 1 : arg(9);

  $form['sel_from_day'] = array(
    '#type' => 'select',
    '#title' => t('Select From Day, Week, Month and Year'),
    '#default_value' => $var_from_day,
    '#options' => _get_days_options(),
  );
  $form['sel_from_month'] = array(
    '#type' => 'select',
    '#default_value' => $var_from_mon,
    '#options' => _get_months_options(),
  );
  $form['sel_from_year'] = array(
    '#type' => 'select',
    '#default_value' => $var_from_year,
    '#options' => _get_years_options(),
  );
  $form['sel_to_day'] = array(
    '#type' => 'select',
    '#title' => t('Select To Day, Week, Month and Year'),
    '#default_value' => $var_to_day,
    '#options' => _get_days_options(),
  );
  $form['sel_to_month'] = array(
    '#type' => 'select',
    '#default_value' => $var_to_mon,
    '#options' => _get_months_options(),
  );
  $form['sel_to_year'] = array(
    '#type' => 'select',
    '#default_value' => $var_to_year,
    '#options' => _get_years_options(),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Go'),
  );

  return $form;
}

/**
 * Implementation of hook_validate().
 */
function daily_trend_form_validate($form, &$form_state) {
  $from_day   = $form_state['values']['sel_from_day'];
  $from_mon   = $form_state['values']['sel_from_month'];
  $from_year  = $form_state['values']['sel_from_year'];
  $to_day     = $form_state['values']['sel_to_day'];
  $to_mon     = $form_state['values']['sel_to_month'];
  $to_year    = $form_state['values']['sel_to_year'];

  if ($from_year > $to_year) {
    form_set_error('sel_from_year', t("'From Year' can't be greater than 'To Year'"));
  }
  if ($from_year == $to_year && $from_mon > $to_mon) {
    form_set_error('sel_from_month', t("'From Month' can't be greater than 'To Month'"));
  }
  if ($from_year == $to_year && $from_mon == $to_mon && $from_day > $to_day) {
    form_set_error('sel_from_day', t("'From Day' can't be greater than 'To Day'"));
  }
}

/**
 * Implementation of hook_submit().
 */
function daily_trend_form_submit($form, &$form_state) {
  $from_mon = $form_state['values']['sel_from_month'];
  $from_year = $form_state['values']['sel_from_year'];
  $to_mon = $form_state['values']['sel_to_month'];
  $to_year = $form_state['values']['sel_to_year'];
  $from_day = $form_state['values']['sel_from_day'];
  $to_day = $form_state['values']['sel_to_day'];
  drupal_goto('admin/reports/user_trend/daily/'
    . $from_mon . '/' . $from_year . '/' . $to_mon . '/' . $to_year
    . '/' . $from_day . '/' . $to_day);
}

/**
 * Generates report: builds and executes query, returns HTML output
 * @return string
 */
function daily_trend_report() {
  $var_from_mon   = arg(4) == '' ? date('m') : arg(4);
  $var_from_year  = arg(5) == '' ? date('Y') : arg(5);
  $var_to_mon     = arg(6) == '' ? date('m') : arg(6);
  $var_to_year    = arg(7) == '' ? date('Y') : arg(7);
  $from_day       = arg(8) == '' ? '01' : arg(8);
  $to_day         = arg(9) == '' ? '01' : arg(9);

  $from_year_month_day = $var_from_year . '-' . $var_from_mon . '-' . $from_day;
  $varFromTimestamp = strtotime($from_year_month_day . '  00:00:00');
  $to_year_month_day = $var_to_year . '-' . $var_to_mon . '-' . $to_day;
  $varToTimestamp = strtotime($to_year_month_day . '  23:59:00');

  $header[] = array('data' => t('Day'));
  $header[] = array('data' => t('Total User Logins'));

  $query = db_select('user_trend', 'ut');
  $query->addExpression("FROM_UNIXTIME(login_time, '%d, %M, %Y')", 'login_date');
  $query->addExpression("COUNT(*)", 'value_count');
  $query->where("login_time >= :from AND login_time <= :to",
    array(':from' => $varFromTimestamp, ':to' => $varToTimestamp))
    ->groupBy("FROM_UNIXTIME(login_time,'%d, %M, %Y')")
    ->orderBy('login_time');
  $result = $query->extend('PagerDefault')->limit(5)->execute();

  $rows = _get_rows_from_result($result);
  $output = theme('table', array('header' => $header, 'rows' => $rows));
  return $output . theme('pager');
}


/*****************************************************************************/
/**
 * Build the array for Year dropdown option
 *
 * @return array
 */
function _get_years_options() {
  for ($i = UT_YEAR_START; $i <= UT_YEAR_END; $i++) {
    $options[$i] = $i;
  }
  return $options;
}

/**
 * Build the array for Month dropdown option
 *
 * @return array
 */
function _get_months_options() {
  return array(
    '01' => t('Jan'),
    '02' => t('Feb'),
    '03' => t('Mar'),
    '04' => t('Apr'),
    '05' => t('May'),
    '06' => t('Jun'),
    '07' => t('Jul'),
    '08' => t('Aug'),
    '09' => t('Sep'),
    '10' => t('Oct'),
    '11' => t('Nov'),
    '12' => t('Dec'),
  );
}

/**
 * Build the array for Week dropdown option
 *
 * @return array
 */
function _get_weeks_options() {
  $week = t('Week');
  return array(
    '01' => $week . ' 1',
    '02' => $week . ' 2',
    '03' => $week . ' 3',
    '04' => $week . ' 4',
  );
}

/**
 * Build the array for Days dropdown option
 *
 * @return array
 */
function _get_days_options() {
  $day  = t('Day');
  $days = array();
  for ($i = 1; $i <= 31; $i++) {
    $days[$i] = $day . ' ' . $i;
  }
  return $days;
}

/**
 * Returns first day of given date
 *
 * @param string $week
 * @param string $mon
 * @param string $yr
 * @return bool|string
 */
function _firstOfWeek($week = '', $mon = '', $yr = '') {
  $week   = $week == '' ? '01' : $week;
  $month  = $mon == '' ? date('m') : $mon;
  $year   = $yr == '' ? date('Y') : $yr;
  $day    = _get_day_of_week($week);

  return date("d", strtotime($month . '/' . $day . '/' . $year . ' 00:00:00'));
}

/**
 * Returns last day of given date
 *
 * @param string $week
 * @param string $mon
 * @param string $yr
 * @return bool|string
 */
function _lastOfWeek($week = '', $mon = '', $yr = '') {
  $week   = $week == '' ? '01' : $week;
  $month  = $mon == '' ? date('m') : $mon;
  $year   = $yr == '' ? date('Y') : $yr;
  $day    = _get_day_of_week($week, FALSE);

  return date("d", strtotime($month . '/' . $day . '/' . $year . ' 00:00:00'));
}

/**
 * Returns first/last day of given week
 * @param $week
 * @param bool $first
 * @return string
 */
function _get_day_of_week($week, $first = TRUE) {
  switch ($week) {
    case '01':
      $day_from = '01';
      $day_to = '07';
      break;
    case '02':
      $day_from = '08';
      $day_to = '14';
      break;
    case '03':
      $day_from = '14';
      $day_to = '21';
      break;
    case '04':
      $day_from = '21';
      $day_to = '31';
      break;
  }
  return $first ? $day_from : $day_to;
}

/**
 * Returns last day of month
 *
 * @param string $mon
 * @param string $yr
 * @return bool|string
 */
function _firstOfMonth($mon = '', $yr = '') {
  $month  = $mon == '' ? date('m') : $mon;
  $year   = $yr == '' ? date('Y') : $yr;
  return date("d", strtotime($month . '/01/' . $year . ' 00:00:00'));
}

/**
 * Returns last day of month
 *
 * @param string $mon
 * @param string $yr
 * @return bool|string
 */
function _lastOfMonth($mon = '', $yr = '') {
  $month  = $mon == '' ? date('m') : $mon;
  $year   = $yr == '' ? date('Y') : $yr;
  return date("d", strtotime('-1 second', strtotime('+1 month', strtotime($month . '/01/' . $year . ' 00:00:00'))));
}

/**
 * Returns array of first and last day of quarter
 *
 * @param string $quarterValue
 * @param string $yearValue
 * @return array
 */
function _get_quarter_period($quarterValue = '', $yearValue = '') {
  $quarterArray = array();
  switch ($quarterValue) {
    case 1:
      $quarterArray['start'] = strtotime($yearValue . '-01-' . _firstOfMonth(01, $yearValue) . '  00:00:00');
      $quarterArray['end'] = strtotime($yearValue . '-03-' . _lastOfMonth(01, $yearValue) . '  23:59:00');
      break;
    case 2:
      $quarterArray['start'] = strtotime($yearValue . '-04-' . _firstOfMonth(01, $yearValue) . '  00:00:00');
      $quarterArray['end'] = strtotime($yearValue . '-06-' . _lastOfMonth(01, $yearValue) . '  23:59:00');
      break;
    case 3:
      $quarterArray['start'] = strtotime($yearValue . '-07-' . _firstOfMonth(01, $yearValue) . '  00:00:00');
      $quarterArray['end'] = strtotime($yearValue . '-09-' . _lastOfMonth(01, $yearValue) . '  23:59:00');
      break;
    case 4:
      $quarterArray['start'] = strtotime($yearValue . '-10-' . _firstOfMonth(01, $yearValue) . '  00:00:00');
      $quarterArray['end'] = strtotime($yearValue . '-12-' . _lastOfMonth(01, $yearValue) . '  23:59:00');
      break;
  }
  return $quarterArray;
}
