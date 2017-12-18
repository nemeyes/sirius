using FirstFloor.ModernUI.Presentation;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace sirius.app.server.arbitrator
{
    public class AttendantsSettingsViewModel
        : NotifyPropertyChanged, IDataErrorInfo
    {
        private string maximumInstanceCount = "200";


        public string MaximumInstanceCount
        {
            get { return this.maximumInstanceCount; }
            set
            {
                if (this.maximumInstanceCount != value) {
                    this.maximumInstanceCount = value;
                    OnPropertyChanged("MaximumInstanceCount");
                }
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
                if (columnName == "MaximumInstanceCount")
                {
                    return string.IsNullOrEmpty(this.maximumInstanceCount) ? "Required value" : null;
                }
                return null;
            }
        }
    }
}
