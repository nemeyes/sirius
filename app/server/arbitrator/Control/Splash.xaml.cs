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
using sirius.app.server.arbitrator;
using System.Runtime.InteropServices;
using System.Windows.Threading;

namespace sirius.app.server.arbitrator.Control
{
    /// <summary>
    /// Interaction logic for ControlsStylesProgressBar.xaml
    /// </summary>
    public partial class Splash : UserControl
    {
        public static Splash handle;
        public Splash()
        {
            handle = this;
            InitializeComponent();
        }
        public void update_progress_bar(double percent)
        {
            Dispatcher.Invoke(DispatcherPriority.Normal,
            new Action
            (
                delegate()
                    {                        
                        progress.Value = percent / 100;
                    }
            ));
        } 
    }
}
