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

namespace sirius.app.server.arbitrator.Settings
{
    /// <summary>
    /// Interaction logic for Configuration.xaml
    /// </summary>
    /// 
    public static class media_setting
    {
        public static MediaSettings handle;
    }
    public partial class MediaSettings : UserControl
    {
        public MediaSettings()
        {
            media_setting.handle = this;
            InitializeComponent();

            this.Loaded += OnLoaded;
        }

        void OnLoaded(object sender, RoutedEventArgs e)
        {
            // select first control on the form
            Keyboard.Focus(this.ImageCodec);
        }
        private void media_set_apply_click(object sender, RoutedEventArgs e)
        {                   
            
        }
    }
}
