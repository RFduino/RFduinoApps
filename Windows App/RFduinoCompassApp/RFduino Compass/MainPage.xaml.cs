//
// Copyright (c) 2015, RF Digital Corp.
// All rights reserved.
 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    1. Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//
//    2. Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation 
//       and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY 
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
using RFduino_Compass.Common;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.Devices.Enumeration;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using System.Diagnostics;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;
using Windows.UI.Popups;
using Windows.UI.Xaml.Media.Animation;
using System.Threading;
using Windows.Devices.Bluetooth;

// The Basic Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234237

namespace RFduino_Compass
{
    /// <summary>
    /// A basic page that provides characteristics common to most applications.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        GattCharacteristic readCharacteristic;
        GattCharacteristic writeCharacteristic;
        private NavigationHelper navigationHelper;
        private ObservableDictionary defaultViewModel = new ObservableDictionary();
        private Timer Timer;

        /// <summary>
        /// This can be changed to a strongly typed view model.
        /// </summary>
        public ObservableDictionary DefaultViewModel
        {
            get { return this.defaultViewModel; }
        }

        /// <summary>
        /// NavigationHelper is used on each page to aid in navigation and 
        /// process lifetime management
        /// </summary>
        public NavigationHelper NavigationHelper
        {
            get { return this.navigationHelper; }
        }

        /// <summary>
        /// The last heading reading received from the RFduino Compass
        /// </summary>
        public static readonly DependencyProperty HeadingProperty = DependencyProperty.Register("Heading", typeof(double), typeof(MainPage), new PropertyMetadata(default(double)));
        public double Heading
        {
            get { return (double)GetValue(HeadingProperty); }
            set { SetValue(HeadingProperty, value); }
        }

        public MainPage()
        {
            this.InitializeComponent();
            this.navigationHelper = new NavigationHelper(this);
            this.navigationHelper.LoadState += navigationHelper_LoadState;
            this.navigationHelper.SaveState += navigationHelper_SaveState;
        }

        /// <summary>
        /// Populates the page with content passed during navigation. Any saved state is also
        /// provided when recreating a page from a prior session.
        /// </summary>
        /// <param name="sender">
        /// The source of the event; typically <see cref="NavigationHelper"/>
        /// </param>
        /// <param name="e">Event data that provides both the navigation parameter passed to
        /// <see cref="Frame.Navigate(Type, Object)"/> when this page was initially requested and
        /// a dictionary of state preserved by this page during an earlier
        /// session. The state will be null the first time a page is visited.</param>
        private void navigationHelper_LoadState(object sender, LoadStateEventArgs e)
        {
            Debug.WriteLine("LoadState");
            configureRFCompass();
        }

        /// <summary>
        /// Preserves state associated with this page in case the application is suspended or the
        /// page is discarded from the navigation cache.  Values must conform to the serialization
        /// requirements of <see cref="SuspensionManager.SessionState"/>.
        /// </summary>
        /// <param name="sender">The source of the event; typically <see cref="NavigationHelper"/></param>
        /// <param name="e">Event data that provides an empty dictionary to be populated with
        /// serializable state.</param>
        private void navigationHelper_SaveState(object sender, SaveStateEventArgs e)
        {
            Debug.WriteLine("SaveState");
            if(this.Timer != null)
            {
                this.Timer.Dispose();
                this.Timer = null;
            }
        }

        #region NavigationHelper registration

        /// The methods provided in this section are simply used to allow
        /// NavigationHelper to respond to the page's navigation methods.
        /// 
        /// Page specific logic should be placed in event handlers for the  
        /// <see cref="GridCS.Common.NavigationHelper.LoadState"/>
        /// and <see cref="GridCS.Common.NavigationHelper.SaveState"/>.
        /// The navigation parameter is available in the LoadState method 
        /// in addition to page state preserved during an earlier session.

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            navigationHelper.OnNavigatedTo(e);
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            navigationHelper.OnNavigatedFrom(e);
        }

        #endregion

        /// <summary>
        /// Check if there is a device exposing the Compass Service.
        /// If so, register a callback method which is notified
        /// whenever the compass sends a new heading value.
        /// If a Compass Service isn't found, notify the user.
        /// </summary>
        private async void configureRFCompass()
        {
            const string compassServiceUUID = "b329392a-fbcd-49aa-a823-3e87680ac33b";
            const string readCharacteristicUUID = "b329392b-fbcd-49aa-a823-3e87680ac33b";
            const string writeCharacteristicUUID = "b329392c-fbcd-49aa-a823-3e87680ac33b";

            var rfduinoCompassService = await findBLEServiceAsync(compassServiceUUID);

            if(rfduinoCompassService != null)
            {
                //
                // Found! Register for notifications of heading changes
                //
                this.readCharacteristic = await registerCharacteristicChangedCallback(rfduinoCompassService, 
                    readCharacteristicUUID, 
                    headingValueChanged);

                //
                // Save the write characteristic to be used for writing to the compass
                //
                var characteristics = rfduinoCompassService.GetCharacteristics(new Guid(writeCharacteristicUUID));
                if (characteristics.Count != 0)
                {
                    this.writeCharacteristic = characteristics[0];
                }

                //
                // Work around for stability issues
                //
                TypedEventHandler<GattCharacteristic, GattValueChangedEventArgs> handler = headingValueChanged;
                if(this.Timer == null)
                {
                    Timer = new System.Threading.Timer(TimerCallback, handler, 5000, 5000);
                }
            }
            else
            {
                //
                // Couldn't find a compass service.
                // Notify User. There may be other error handling you want to do
                //
                var messageDialog = new MessageDialog(
                            "A compass wasn't found. " + 
                            "Have you paired the compass in Settings?");
                await messageDialog.ShowAsync();
            }
        }

        /// <summary>
        /// Find a device which exposed the given Service UUID. If there
        /// is such a device, return the Service from that device
        /// 
        /// The device must already be attached and paired with the device
        /// this application is running on.
        /// If more than one currently connected device exposes the given
        /// Service, the first one returned from the System is returned to
        /// the caller.
        /// </summary>
        /// <param name="serviceUUID">The UUID of a Service to find in a connected device</param>
        /// <returns>A GattDeviceService for the given Service or null if none found</returns>
        private async Task<GattDeviceService> findBLEServiceAsync(string serviceUUID)
        {
            // 
            // Find known devices which implement the given service UUID
            // The returned value is a device enumeration, but it really
            // seems to be a list of Services.
            //
            var devices = await Windows.Devices.Enumeration.DeviceInformation.FindAllAsync(
                GattDeviceService.GetDeviceSelectorFromUuid(new Guid(serviceUUID)));

            if (devices.Count == 0)
                return null;

            var service = await GattDeviceService.FromIdAsync(devices[0].Id);
            return service;
        }

        /// <summary>
        /// Register a ValueChanged callback on a device Characteristic.
        /// The Characteristic must be readable and send Notifications.
        /// </summary>
        /// <param name="service">The Service containing the Characteristic</param>
        /// <param name="readCharacteristicUUID">The UUID of the Characteristic</param>
        /// <param name="valueChangedHandler">The event handler callback</param>
        /// <returns>A GattCharacteristic for the which the event handler was 
        /// registered or null if the Characteristic wasn’t found</returns>   
        private async Task<GattCharacteristic> registerCharacteristicChangedCallback(GattDeviceService service,
            string readCharacteristicUUID, 
            TypedEventHandler<GattCharacteristic, GattValueChangedEventArgs> valueChangedHandler)
        {
            Debug.Assert(service != null, "Null service passed as argument.");
            Debug.Assert(readCharacteristicUUID != null, "Null Characteristic UUID pass as argument.");

            //Obtain the characteristic we want to interact with
            var characteristics = service.GetCharacteristics(new Guid(readCharacteristicUUID));
            if (characteristics.Count == 0) return null;

            var characteristic = characteristics[0];

            //Subscribe to value changed event
            characteristic.ValueChanged += valueChangedHandler;

            //Set configuration to notify  
            await characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(
                GattClientCharacteristicConfigurationDescriptorValue.Notify);

            return characteristic;
        }

        /// <summary>
        /// Callback when the read characteristic changes.
        /// 
        /// This method expects a single 32 bit integer to be received.
        /// </summary>
        /// <param name="sender">The characteristic upon which that change occurred</param>
        /// <param name="args">The event arguments</param>
        private async void headingValueChanged(GattCharacteristic sender, 
            GattValueChangedEventArgs args)
        {
            try
            {
                Debug.Assert(args.CharacteristicValue.Length == 4,
                    string.Format("Characteristic length of {0} isn't the expected length of 4",
                    args.CharacteristicValue.Length));

                var bytes = args.CharacteristicValue.ToArray();
                await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(CoreDispatcherPriority.Normal,
                () =>
                {
                    //
                    // Don't read or write the heading property while not on the main thread
                    //
                    //
                    // Don't read heading property while not on the main thread
                    //
                    var currentHeading = this.Heading;

                    var newHeading = bytes[0] +
                                     (bytes[1] << 8) +
                                     (bytes[2] << 16) +
                                     (bytes[3] << 24);

                    double delta = angleDifference(currentHeading, newHeading);

                    double nextHeading = currentHeading + delta;

                    //Needed to prevent an error
                    this.CompassNeedleAnimation.From = currentHeading;
                    this.CompassNeedleAnimation.To = nextHeading;

                    Storyboard sb = (Storyboard)CompassNeedle.Resources["spin"];
                    sb.Begin();

                    // Update the text
                    this.CurrentHeadingTextBlock.Text = string.Format("{0}°", newHeading);

                    // Save the new heading
                    this.Heading = newHeading;
                });
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
        }

        /// <summary>
        /// Return the shortest signed difference between the two given andles.
        /// 
        /// The sign of the difference defines the direction traveled to get
        /// from the first angle to the second where clockwise is positive.
        ///    If x = 10 and y = 90, the returned value is 80
        ///    If x = 90 and y = 10, the returned value is -80
        /// 
        /// When it is interesting is when the "origin" (0 0r 360) is crossed
        ///    if x = 10 and y = 350, the returned value is -20
        ///    if x = 350 and y = 10, the returned value is 20
        ///    
        /// Note: The method doesn't handle multiple trips around the unit circle.
        ///       i.e. inpot angles are expected between 0-360.
        /// </summary>
        /// <param name="x">The first angle in degrees</param>
        /// <param name="y">The second angle in degrees</param>
        /// <returns>The difference between the two given angles in degrees</returns>
        private double angleDifference(double x, double y)
        {
            double C360 = 360.000000000;

            double arg;
            arg = Math.IEEERemainder(y - x, C360);
            if (arg < 0) arg = arg + C360;
            if (arg > 180) arg = arg - C360;
            return (arg);
        }

        /// <summary>
        /// Callback method to watchdog timer. Reset compass configuration
        /// </summary>
        /// <param name="state">Not used.</param>
        private void TimerCallback(object state)
        {
            Debug.WriteLine("Timer called handler {0}", state);
            TypedEventHandler<GattCharacteristic, GattValueChangedEventArgs> valueChangedHandler = (TypedEventHandler<GattCharacteristic, GattValueChangedEventArgs>)state;
            if(this.readCharacteristic != null)
            {
                this.readCharacteristic.ValueChanged -= valueChangedHandler;
            }
            configureRFCompass();
        }

        /// <summary>
        /// The calibrate button was clicked. Send a calibration request
        /// to the compass
        /// </summary>
        /// <param name="sender">The button which was clicked</param>
        /// <param name="e">The event arguments</param>
        private async void Calibrate_ButtonClick(object sender, RoutedEventArgs e)
        {
            if (this.writeCharacteristic == null) return;

            try
            {
                byte value = 0xAA;
                await this.writeCharacteristic.WriteValueAsync((new byte[] { value }).AsBuffer());
            } catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
        }
    }
}
