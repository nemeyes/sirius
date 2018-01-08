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
        private string attendant_instance_count = "500";
        private string attendant_url = "https://www.youtube.com/tv";
        private string attendant_creation_delay = "1000";
        
        public string AttendantInstanceCount
        {
            get { return this.attendant_instance_count; }
            set
            {          
                this.attendant_instance_count = value;
                OnPropertyChanged("AttendantInstanceCount");             
            }
        }

        public string AttendantUrl
        {
            get { return this.attendant_url; }
            set
            {
                this.attendant_url = value;
                OnPropertyChanged("AttendantUrl");               
            }
        }

        public string AttendantCreationDelay
        {
            get { return this.attendant_creation_delay; }
            set
            {
                this.attendant_creation_delay = value;
                OnPropertyChanged("AttendantCreationDelay");
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
                if (columnName == "AttendantInstanceCount")
                {
                    return string.IsNullOrEmpty(this.attendant_instance_count) ? "Required value" : null;
                }

                if (columnName == "AttendantUrl")
                {
                    return string.IsNullOrEmpty(this.attendant_url) ? "Required value" : null;
                }

                if (columnName == "AttendantCreationDelay")
                {
                    return string.IsNullOrEmpty(this.attendant_creation_delay) ? "Required value" : null;
                }
                return null;
            }
        }
    }
}
