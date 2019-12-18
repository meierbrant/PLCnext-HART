import ko from 'knockout';
import templateMarkup from 'text!./hart-device.html';

class HartDevice {
    constructor(params) {
        this.message = ko.observable('Hello from the hart-device component!');
    }
    
    dispose() {
        // This runs when the component is torn down. Put here any logic necessary to clean up,
        // for example cancelling setTimeouts or disposing Knockout subscriptions/computeds.
    }
}

export default { viewModel: HartDevice, template: templateMarkup };
