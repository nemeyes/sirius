﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using sirius.app.server.arbitrator.Settings;
using sirius.app.server.arbitrator.Controls;
using FirstFloor.ModernUI.Windows.Controls;

namespace sirius.app.server.arbitrator.Control
{
    /// <summary>
    /// Interaction logic for Status.xaml
    /// </summary>
    public partial class Status : Page 
    {
        public static Status handle;
        public Status()
        {
            handle = this;
            InitializeComponent();
            
            this.DataContext = new ChartViewModel();

            chart_view_model.handle.cpu_chart.Category = SettingValue.Instance().cpu + "\r\n\r\n";
            chart_view_model.handle.memory_chart.Category = "Memory Installed : " + SettingValue.Instance().memory + "\r\n\r\n";

            start_button.IsEnabled = true;
            stop_button.IsEnabled = false;

            //if (SettingValue.Instance().enable_auto_start)
            //{
            //   start_button.IsEnabled = false;
            //   stop_button.IsEnabled = false;
            //}
            //else
            //{
            //    start_button.IsEnabled = true;
            //    stop_button.IsEnabled = false;
            //}  
        }
    
        private void ShellView_Loaded(object sender, RoutedEventArgs e)
        {
            //Matrix m = PresentationSource.FromVisual(Application.Current.MainWindow).CompositionTarget.TransformToDevice;
            //double dx = m.M11;
            //double dy = m.M22;

            //ScaleTransform s = (ScaleTransform)mainGrid.LayoutTransform;
            //s.ScaleX = 1 / dx;
            //s.ScaleY = 1 / dy;
        }
      
        private void start_button_click(object sender, RoutedEventArgs e)
        {
            SettingValue setting_value = SettingValue.Instance();
            Uri uriResult;
            if (!Uri.TryCreate(setting_value.url, UriKind.Absolute, out uriResult))
            {
                var v = new ModernDialog
                {
                    Title = "Error",
                    Content = "CODE: INVALID URL \n\n [SETTINGS] -> [ATTENDANT] -> [URL]"
                };
                v.ShowDialog();
                return;                
            }

            if (uriResult.Scheme == Uri.UriSchemeFile)
            {
                System.IO.FileInfo fi = new System.IO.FileInfo(setting_value.url);
                if (!fi.Exists)
                {
                    var v = new ModernDialog
                    {
                        Title = "Error",
                        Content = "CODE: INVALID URL \n\n [SETTINGS] -> [ATTENDANT] -> [URL]"
                    };
                    v.ShowDialog();
                    return;
                }
            }

            Dispatcher.Invoke(DispatcherPriority.Normal,
            new Action
            (
                delegate ()
                {
                    start_button.IsEnabled = false;
                    stop_button.IsEnabled = false;
                }
            ));
            sirius_arbitrator.controller.start();
        }

        private void stop_button_click(object sender, RoutedEventArgs e)
        {
            Dispatcher.Invoke(DispatcherPriority.Normal,
            new Action
            (
                delegate ()
                {
                    start_button.IsEnabled = true;
                    stop_button.IsEnabled = false;
                }
            ));
            sirius_arbitrator.controller.stop();
        }
        public void update_usage(double cpu_usage, double memory_usage)
        {
            Dispatcher.Invoke(DispatcherPriority.Normal,
            new Action
            (
                delegate ()
                {                  
                    chart_view_model.handle.cpu_chart.Number = (float)cpu_usage;
                    chart_view_model.handle.memory_chart.Number = (float)memory_usage;                          
                }
            ));
        }

        public void update_connect_count(int max_cnt,int conn_cnt)
        {
            Dispatcher.Invoke(DispatcherPriority.Normal,
            new Action
            (
                delegate ()
                {                    
                    connect_count.Content = conn_cnt + " / " + max_cnt;
                }
            ));
        }
    }
}
