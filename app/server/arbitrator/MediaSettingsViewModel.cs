using FirstFloor.ModernUI.Presentation;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace sirius.app.server.arbitrator
{
    public class MediaSettingsViewModel
        : NotifyPropertyChanged, IDataErrorInfo
    {
        private Int32 imageCompressionLevel = 5;
        private string imageWidth = "1280";
        private string imageHeight = "720";
        private string imageBlockWidth = "128";
        private string imageBlockHeight = "72";

        //private string streamerPortnumber = "7000";
        //private string keepAliveTimeout = "5";

        public Int32 ImageCompressionLevel
        {
            get { return this.imageCompressionLevel; }
            set
            {
                if (this.imageCompressionLevel != value) {
                    this.imageCompressionLevel = value;
                    OnPropertyChanged("ImageCompressionLevel");
                }
            }
        }

        public string ImageWidth
        {
            get { return this.imageWidth; }
            set
            {
                if (this.imageWidth != value)
                {
                    this.imageWidth = value;
                    OnPropertyChanged("ImageWidth");
                }
            }
        }

        public string ImageHeight
        {
            get { return this.imageHeight; }
            set
            {
                if (this.imageHeight != value)
                {
                    this.imageHeight = value;
                    OnPropertyChanged("ImageHeight");
                }
            }
        }

        public string ImageBlockWidth
        {
            get { return this.imageBlockWidth; }
            set
            {
                if (this.imageBlockWidth != value)
                {
                    this.imageBlockWidth = value;
                    OnPropertyChanged("ImageBlockWidth");
                }
            }
        }

        public string ImageBlockHeight
        {
            get { return this.imageBlockHeight; }
            set
            {
                if (this.imageBlockHeight != value)
                {
                    this.imageBlockHeight = value;
                    OnPropertyChanged("ImageBlockHeight");
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
                if (columnName == "ImageCompressionLevel") {
                    return this.imageCompressionLevel==0 ? "Required value" : null;
                }
                if (columnName == "ImageWidth")
                {
                    return string.IsNullOrEmpty(this.imageWidth) ? "Required value" : null;
                }
                if (columnName == "ImageHeight")
                {
                    return string.IsNullOrEmpty(this.imageHeight) ? "Required value" : null;
                }
                if (columnName == "ImageBlockWidth")
                {
                    return string.IsNullOrEmpty(this.imageBlockWidth) ? "Required value" : null;
                }
                if (columnName == "ImageBlockHeight")
                {
                    return string.IsNullOrEmpty(this.imageBlockHeight) ? "Required value" : null;
                }
                return null;
            }
        }
    }
}
