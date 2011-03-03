<?php

/**
 * @file
 *   Implementation of abstract class ChartsGraphsCanvas for Flot library.
 *
 */

/**
 * Implementation of abstract class ChartsGraphsCanvas for Flot library.
 */
class ChartsGraphsFlot extends ChartsGraphsCanvas {

  var $width = 450;
  var $height = 200;
  var $title = '';
  var $title_font_size = 32;
  var $xaxis = "";
  var $yaxis = "";
  var $group_label = "";
  var $grouping = "";
  /**
   * Parameters set directly by the user.
   *
   * @var <array>
   */
  var $parameters = array();
  
  /**
   * Optional parameter.
   * 
   * If set to TRUE, will cause Flot to pick the opposite data orientation to
   * the one it would have picked automatically. This orientation defines if
   * each column or each row is one data serie.
   * 
   * @var <bool> 
   */
  var $transpose = NULL;

  function _get_translated_chart_type() {}

  protected function _initialize_final_parameters($chart_id, $transpose) {
    $parameters = array();
    $parameters['marker_font_size'] = 20;
    $parameters['hide_legend'] = 'true';
    $parameters['hide_title'] = 'false';
    $parameters['sort'] = 'false';
    $parameters['title_font_size'] = (int) $this->title_font_size;
    $parameters['tooltips'] = 'true';

    /**
     * Applying user defined min, max and step for y axis values.
     */
    if ($this->y_min) {
      $parameters['minimum_value'] = $this->y_min;
    }
    if ($this->y_max)   {
      $parameters['maximum_value'] = $this->y_max;
    }
    if ($this->y_step)  {
      $parameters['y_axis_increment'] = $this->y_step;
    }

    $series_colours = "['" . implode("', '", $this->series_colours) . "']";
    $marker_colour = '#aea9a9';
    $font_colour = '#000000';
    $background_colour = isset($this->colour) ?
      array($this->colour, $this->colour) :
      array(
        '#d1edf5',
        '#ffffff',
      );
    $background_colour = "['" . implode("', '", $background_colour) . "']";
    $set_theme = sprintf(
      "{
        colors: %s,
        marker_color: '%s',
        font_color: '%s',
        background_colors: %s
      }",
      $series_colours,
      $marker_colour,
      $font_colour,
      $background_colour
    );
    $parameters['set_theme'] = array($set_theme);

    $parameters['data_from_table'] = array(
      '"' . $chart_id . '"',
      ($transpose) ? 'true' : 'false'
    );

    if (is_array($this->parameters)) {
      foreach ($this->parameters as $user_parameter_key => $user_parameter) {
        $parameters[$user_parameter_key] = $user_parameter;
      }
    }
    $this->final_parameters = $parameters;
  }

  protected function _get_encoded_parameters() {
    $output = "\n";
    foreach ($this->final_parameters as $key => $value) {
      if (is_array($value)) {
        $function_parameters = '';
        foreach ($value as $val) {
          $function_parameters .= sprintf(', %s', $val);
        }
        if (strlen($function_parameters) > 0) {
          $function_parameters = substr($function_parameters, 2);
        }
        $output .= sprintf("\tg.%s(%s);\n", $key, $function_parameters);
      }
      else {
        $output .= sprintf("\tg.%s = %s;\n", $key, $value);
      }
    }
    return $output;
  }
  
 public function utime_to_jstime($timestamp) {
  $original_date = new DateTime(NULL, new DateTimeZone(date_default_timezone_name()));
  $original_date->setTimestamp($timestamp);
  $adjusted_date = new DateTime($original_date->format('Y-m-d H:i:s'), new DateTimeZone('UTC'));
  $newtime = $adjusted_date->format('U');
  // Convert the Unix timestamp to milliseconds so Javascript can understand it
  $newtime = $newtime * 1000;
  return $newtime;
 }
  /**
   * Function that renders data.
   */
  public function get_chart() {
    $flot_js_files = $this->get_flot_js_files();

    foreach ($flot_js_files as $file_path) {
      drupal_add_js($file_path);
    }

    $flot_path = drupal_get_path('module', 'charts_graphs_flot');
    drupal_add_css($flot_path . '/charts_graphs_flot.css');

    $x_labels = $this->x_labels;
    $series = $this->series;
    $chart_id = 'flotchart-' . charts_graphs_chart_id_generator();
    $data = array();

    $data[] = sprintf('<div id="%s" class="flot-data-table">', $chart_id);
    $i = 0;
    $xaxis = $this->xaxis;
    $yaxis = $this->yaxis;
    $keys = array_keys($series);
    foreach($series[$keys[0]] as $value) {
      $data_points[] = array($this->utime_to_jstime($series[$xaxis][$i]), $series[$yaxis][$i]);
      $i++;
    }
    $flot_data = array("label" => $this->group_label, "data" => $data_points);
    $data[] = "[" . drupal_to_js($flot_data) . "]";

    $data[] = "</div>\n";


    $this->_initialize_final_parameters($chart_id, $transpose);

    $canvas = '
      <div id="%chart_id-graph" style="width:%width; height:%height; position: relative;"></div>';
    $canvas = strtr(
      $canvas,
      array(
        '%chart_id' => $chart_id,
        '%width' => $this->width,
        '%height' => $this->height,
    ));
    $html = implode('', $data);

    $javascript = '
        var ChartsAndGraphs = ChartsAndGraphs || {};
        // This currently runs as many times as there are displays!
        ChartsAndGraphs.init = function() {
          var data = [];
          var i = 2;
          
          while($("#flotchart-" + i).text()) {
            var set = eval($("#flotchart-" + i).text());
            data.push(set[0]);
            if("#flotchart-" + i != "#flotchart-2") {
             $("#flotchart-" + i + "-graph").hide();
             $("#flotchart-" + i).hide();
            }
            i = i + 2;
          };
          $("#flotchart-2").hide();
          $.plot($("#flotchart-2-graph"), data, {"xaxis": { "mode": "time" } });
        }
        $(window).load(ChartsAndGraphs.init);

        Drupal.behaviors.ChartsAndGraphs_init = function(context) {
          ChartsAndGraphs.init();
        }
      ';


    $js = strtr(
      $javascript,
      array(
        '%chart_id' => $chart_id,
        '%type' => $this->_get_translated_chart_type(),
        '%width' => $this->width,
        '%height' => $this->height,
    ));


    drupal_add_js($js, 'inline');


    return $html . $canvas; //. $javascript;
  }

  /**
   * Cache list of javascript files for performance.
   */
  function get_flot_js_files() {
    static $js_files = NULL;

    if (is_array($js_files)) {
      return $js_files;
    }

    $flot_path = drupal_get_path('module', 'charts_graphs_flot');
    $flot_files = array_map('basename', glob(dirname(__FILE__) . '/flot/*.js'));

    rsort($flot_files);

    $js_files = array();

    foreach ($flot_files as $flot_file) {
      $file_path = sprintf('%s/flot/%s', $flot_path, $flot_file);
      $js_files[] = $file_path;
    }

    return $js_files;
  }
}
