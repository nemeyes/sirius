using System;
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

namespace sirius.app.server.arbitrator.Control
{
    /// <summary>
    /// Interaction logic for Status.xaml
    /// </summary>
    public partial class Status : Page 
    {
        public static class status_page
        {
            public static Status handle;
        }
        public Status()
        {
            status_page.handle = this;
            InitializeComponent();
            
            this.DataContext = new ChartViewModel();            
        }
    
        private void ShellView_Loaded_1(object sender, RoutedEventArgs e)
        {
            Matrix m = PresentationSource.FromVisual(Application.Current.MainWindow).CompositionTarget.TransformToDevice;
            double dx = m.M11;
            double dy = m.M22;

            ScaleTransform s = (ScaleTransform)mainGrid.LayoutTransform;
            s.ScaleX = 1 / dx;
            s.ScaleY = 1 / dy;
        }
      
        private void start_button_Click(object sender, RoutedEventArgs e)
        {
           loading_bar.IsOpen = true;
           MainWindow.proxy.core.start();
        }

        private void stop_button_Click(object sender, RoutedEventArgs e)
        {
            MainWindow.proxy.core.stop();
        }
        public void on_attendant_load()
        {
            System.Threading.Thread.Sleep(1000 * 1);
            Dispatcher.Invoke(DispatcherPriority.Normal,
                new Action
                (
                    delegate ()
                    {
                        loading_bar.IsOpen = false;
                    }
                ));           
        }
             
        //ChartClass cpu_chart = new ChartClass() { Category = "CPU Usage", Number = 10 };
        //ChartClass momory_chart = new ChartClass() { Category = "Memory Usage", Number = 10 };
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
    }
}
