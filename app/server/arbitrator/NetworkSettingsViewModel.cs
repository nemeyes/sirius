using FirstFloor.ModernUI.Presentation;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace sirius.app.server.arbitrator
{
    public class NetworkSettingsViewModel
        : NotifyPropertyChanged, IDataErrorInfo
    {
        private string controllerPortnumber = "5000";
        private string streamerPortnumber = "7000";
        private string keepAliveTimeout = "5";

        public string ControllerPortnumber
        {
            get { return this.controllerPortnumber; }
            set
            {
                if (this.controllerPortnumber != value) {
                    this.controllerPortnumber = value;
                    OnPropertyChanged("ControllerPortnumber");
                }
            }
        }

        public string StreamerPortnumber
        {
            get { return this.streamerPortnumber; }
            set
            {
                if (this.streamerPortnumber != value) {
                    this.streamerPortnumber = value;
                    OnPropertyChanged("StreamerPortnumber");
                }
            }
        }

        public string KeepAliveTimeout
        {
            get { return this.keepAliveTimeout; }
            set
            {
                if (this.keepAliveTimeout != value)
                {
                    this.keepAliveTimeout = value;
                    OnPropertyChanged("KeepAliveTimeout");
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
                if (columnName == "ControllerPortnumber") {
                    return string.IsNullOrEmpty(this.controllerPortnumber) ? "Required value" : null;
                }
                if (columnName == "StreamerPortnumber") {
                    return string.IsNullOrEmpty(this.streamerPortnumber) ? "Required value" : null;
                }
                if (columnName == "KeepAliveTimeout")
                {
                    return string.IsNullOrEmpty(this.keepAliveTimeout) ? "Required value" : null;
                }
                return null;
            }
        }
    }
}
