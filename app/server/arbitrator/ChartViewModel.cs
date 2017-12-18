using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

#if NETFX_CORE
using Windows.UI.Xaml;
#else

#endif

namespace sirius.app.server.arbitrator
{
    public class ChartViewModel : INotifyPropertyChanged
    {
        public DelegateCommand AddSeriesCommand { get; set; }
        public ObservableCollection<string> ChartTypes { get; set; }
        public List<double> FontSizes { get; set; }
        public List<double> DoughnutInnerRadiusRatios { get; set; }
        public Dictionary<string, De.TorstenMandelkow.MetroChart.ResourceDictionaryCollection> Palettes { get; set; }
        public List<string> SelectionBrushes { get; set; }

        private string selectedChartType = null;
        public string SelectedChartType
        {
            get
            {
                return selectedChartType;
            }
            set
            {
                selectedChartType = value;
                NotifyPropertyChanged("SelectedChartType");
            }
        }

        private object selectedPalette = null;
        public object SelectedPalette
        {
            get
            {
                return selectedPalette;
            }
            set
            {
                selectedPalette = value;
                NotifyPropertyChanged("SelectedPalette");
            }
        }

        private bool darkLayout = false;
        public bool DarkLayout
        {
            get
            {
                return darkLayout;
            }
            set
            {
                darkLayout = value;
                NotifyPropertyChanged("DarkLayout");
                NotifyPropertyChanged("Foreground");
                NotifyPropertyChanged("Background");
                NotifyPropertyChanged("MainBackground");
                NotifyPropertyChanged("MainForeground");
            }
        }

        public string Foreground
        {
            get
            {
                if (darkLayout)
                {
                    return "#FFEEEEEE";
                }
                return "#FF666666";
            }
        }
        public string MainForeground
        {
            get
            {
                if (darkLayout)
                {
                    return "#FFFFFFFF";
                }
                return "#FF666666";
            }
        }
        public string Background
        {
            get
            {
                if (darkLayout)
                {
                    return "#FF333333";
                }
                return "#FFF9F9F9";
            }
        }
        public string MainBackground
        {
            get
            {
                if (darkLayout)
                {
                    return "#FF000000";
                }
                return "#FFEFEFEF";
            }
        }


        private string selectedBrush = null;
        public string SelectedBrush
        {
            get
            {
                return selectedBrush;
            }
            set
            {
                selectedBrush = value;
                NotifyPropertyChanged("SelectedBrush");
            }
        }

        private double selectedDoughnutInnerRadiusRatio = 0.75;
        public double SelectedDoughnutInnerRadiusRatio
        {
            get
            {
                return selectedDoughnutInnerRadiusRatio;
            }
            set
            {
                selectedDoughnutInnerRadiusRatio = value;
                NotifyPropertyChanged("SelectedDoughnutInnerRadiusRatio");
                NotifyPropertyChanged("SelectedDoughnutInnerRadiusRatioString");
            }
        }

        public string SelectedDoughnutInnerRadiusRatioString
        {
            get
            {
                return String.Format("{0:P1}.", SelectedDoughnutInnerRadiusRatio);
            }
        }

        public ChartViewModel()
        {
            LoadPalettes();

            AddSeriesCommand = new DelegateCommand(x => AddSeries());

            ChartTypes = new ObservableCollection<string>();
            ChartTypes.Add("All");
            ChartTypes.Add("Column");
            ChartTypes.Add("StackedColumn");
            ChartTypes.Add("Bar");
            ChartTypes.Add("StackedBar");
            ChartTypes.Add("Pie");
            ChartTypes.Add("Doughnut");
            ChartTypes.Add("Gauge");
            SelectedChartType = ChartTypes.FirstOrDefault();

            FontSizes = new List<double>();
            FontSizes.Add(9.0);
            FontSizes.Add(11.0);
            FontSizes.Add(13.0);
            FontSizes.Add(18.0);
            SelectedFontSize = 11.0;

            DoughnutInnerRadiusRatios = new List<double>();
            DoughnutInnerRadiusRatios.Add(0.90);
            DoughnutInnerRadiusRatios.Add(0.75);
            DoughnutInnerRadiusRatios.Add(0.5);
            DoughnutInnerRadiusRatios.Add(0.25);
            DoughnutInnerRadiusRatios.Add(0.1);
            SelectedDoughnutInnerRadiusRatio = 0.75;

            SelectionBrushes = new List<string>();
            SelectionBrushes.Add("Orange");
            SelectionBrushes.Add("Red");
            SelectionBrushes.Add("Yellow");
            SelectionBrushes.Add("Blue");
            SelectionBrushes.Add("[NoColor]");
            SelectedBrush = SelectionBrushes.FirstOrDefault();

            Series = new ObservableCollection<SeriesData>();

            Errors = new ObservableCollection<ChartClass>();
            Warnings = new ObservableCollection<ChartClass>();

            ObservableCollection<ChartClass> Infos = new ObservableCollection<ChartClass>();

            Errors.Add(new ChartClass() { Category = "Globalization", Number = 75 });
            Errors.Add(new ChartClass() { Category = "Features", Number = 2 });
            Errors.Add(new ChartClass() { Category = "ContentTypes", Number = 12 });
            Errors.Add(new ChartClass() { Category = "Correctness", Number = 83 });
            //Errors.Add(new ChartClass() { Category = "Naming", Number = 80 });
            Errors.Add(new ChartClass() { Category = "Best Practices", Number = 29 });

            Warnings.Add(new ChartClass() { Category = "Globalization", Number = 34 });
            Warnings.Add(new ChartClass() { Category = "Features", Number = 23 });
            Warnings.Add(new ChartClass() { Category = "ContentTypes", Number = 15 });
            Warnings.Add(new ChartClass() { Category = "Correctness", Number = 66 });
            Warnings.Add(new ChartClass() { Category = "Naming", Number = 56 });
            Warnings.Add(new ChartClass() { Category = "Best Practices", Number = 34 });

            Infos.Add(new ChartClass() { Category = "Globalization", Number = 14 });
            Infos.Add(new ChartClass() { Category = "Features", Number = 3 });
            Infos.Add(new ChartClass() { Category = "ContentTypes", Number = 55 });
            Infos.Add(new ChartClass() { Category = "Correctness", Number = 26 });
            Infos.Add(new ChartClass() { Category = "Naming", Number = 3 });
            Infos.Add(new ChartClass() { Category = "Best Practices", Number = 8 });

            Series.Add(new SeriesData() { SeriesDisplayName = "Errors", Items = Errors });
            Series.Add(new SeriesData() { SeriesDisplayName = "Warnings", Items = Warnings });
            Series.Add(new SeriesData() { SeriesDisplayName = "Info", Items = Infos });
        }

        int newSeriesCounter = 1;
        private void AddSeries()
        {
            ObservableCollection<ChartClass> data = new ObservableCollection<ChartClass>();

            data.Add(new ChartClass() { Category = "Globalization", Number = 5 });
            data.Add(new ChartClass() { Category = "Features", Number = 10 });
            data.Add(new ChartClass() { Category = "ContentTypes", Number = 15 });
            data.Add(new ChartClass() { Category = "Correctness", Number = 20 });
            data.Add(new ChartClass() { Category = "Naming", Number = 15 });
            data.Add(new ChartClass() { Category = "Best Practices", Number = 10 });

            Series.Add(new SeriesData() { SeriesDisplayName = "New Series " + newSeriesCounter.ToString(), Items = data });

            newSeriesCounter++;
        }

        private void LoadPalettes()
        {
            Palettes = new Dictionary<string, De.TorstenMandelkow.MetroChart.ResourceDictionaryCollection>();
            Palettes.Add("Default", null);

            var resources = Application.Current.Resources.MergedDictionaries.ToList();
            foreach (var dict in resources)
            {
                foreach (var objkey in dict.Keys)
                {
                    if (dict[objkey] is De.TorstenMandelkow.MetroChart.ResourceDictionaryCollection)
                    {
                        Palettes.Add(objkey.ToString(), dict[objkey] as De.TorstenMandelkow.MetroChart.ResourceDictionaryCollection);
                    }
                }
            }

            SelectedPalette = Palettes.FirstOrDefault();
        }

        private bool isRowColumnSwitched = false;
        public bool IsRowColumnSwitched
        {
            get
            {
                return isRowColumnSwitched;
            }
            set
            {
                isRowColumnSwitched = value;
                NotifyPropertyChanged("IsRowColumnSwitched");
            }
        }

        private bool isLegendVisible = true;
        public bool IsLegendVisible
        {
            get
            {
                return isLegendVisible;
            }
            set
            {
                isLegendVisible = value;
                NotifyPropertyChanged("IsLegendVisible");
            }
        }

        private bool isTitleVisible = true;
        public bool IsTitleVisible
        {
            get
            {
                return isTitleVisible;
            }
            set
            {
                isTitleVisible = value;
                NotifyPropertyChanged("IsTitleVisible");
            }
        }

        private double fontSize = 11.0;
        public double SelectedFontSize
        {
            get
            {
                return fontSize;
            }
            set
            {
                fontSize = value;
                NotifyPropertyChanged("SelectedFontSize");
                NotifyPropertyChanged("SelectedFontSizeString");
            }
        }

        public string SelectedFontSizeString
        {
            get
            {
                return SelectedFontSize.ToString() + "px";
            }
        }

        private object selectedItem = null;
        public object SelectedItem
        {
            get
            {
                return selectedItem;
            }
            set
            {
                selectedItem = value;
                NotifyPropertyChanged("SelectedItem");
            }
        }

        public ObservableCollection<SeriesData> Series
        {
            get;
            set;
        }

        public ObservableCollection<ChartClass> Errors
        {
            get;
            set;
        }

        public ObservableCollection<ChartClass> Warnings
        {
            get;
            set;
        }

        private void NotifyPropertyChanged(string property)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged.Invoke(this, new PropertyChangedEventArgs(property));
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public string ToolTipFormat
        {
            get
            {
                return "{0} in series '{2}' has value '{1}' ({3:P2})";
            }
        }
    }

    public class DelegateCommand : ICommand
    {
        private readonly Predicate<object> _canExecute;
        private readonly Action<object> _execute;

        public event EventHandler CanExecuteChanged;

        public DelegateCommand(Action<object> execute)
            : this(execute, null)
        {
        }

        public DelegateCommand(Action<object> execute,
                       Predicate<object> canExecute)
        {
            _execute = execute;
            _canExecute = canExecute;
        }

        public bool CanExecute(object parameter)
        {
            if (_canExecute == null)
            {
                return true;
            }

            return _canExecute(parameter);
        }

        public void Execute(object parameter)
        {
            _execute(parameter);
        }

        public void RaiseCanExecuteChanged()
        {
            if (CanExecuteChanged != null)
            {
                CanExecuteChanged(this, EventArgs.Empty);
            }
        }
    }
}
