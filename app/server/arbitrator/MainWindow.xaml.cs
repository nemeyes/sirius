using FirstFloor.ModernUI.Windows.Controls;
using FirstFloor.ModernUI.Presentation;
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
using System.Windows.Shapes;

namespace sirius.app.server.arbitrator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : ModernWindow
    {
        private static sirius_arbitrator core;
        public MainWindow()
        {            
            InitializeComponent();   
            //AppearanceManager.Current.ThemeSource = AppearanceManager.DarkThemeSource;
            AppearanceManager.Current.AccentColor = Color.FromRgb(0xf0, 0x96, 0x09);

            core = new sirius_arbitrator(this);
        }
    }
}
