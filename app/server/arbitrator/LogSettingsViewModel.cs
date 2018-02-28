using FirstFloor.ModernUI.Presentation;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace sirius.app.server.arbitrator
{
    public class LogSettingsViewModel
        : NotifyPropertyChanged, IDataErrorInfo
    {
        private string log_root_path = "d:\\log";

        public string LogRootPath
        {
            get { return this.log_root_path; }
            set
            {
                this.log_root_path = value;
                OnPropertyChanged("LogRootPath");
            }
        }     

        public string Error
        {
            get { return null; }
        }

        public string this[string columnName]
        {
            get
            {
                if (columnName == "LogRootPath")
                {
                    return string.IsNullOrEmpty(this.log_root_path) ? "Required value" : null;
                }            
                return null;
            }
        }
    }
}
