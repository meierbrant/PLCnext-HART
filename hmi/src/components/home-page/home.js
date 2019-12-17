import ko from 'knockout';
import homeTemplate from 'text!./home.html';

class HomeViewModel {
    constructor(route) {
        this.message = ko.observable('Welcome to pxc-hart-management!');
    }
    
    doSomething() {
        this.message('You invoked doSomething() on the viewmodel.');
    }
}

export default { viewModel: HomeViewModel, template: homeTemplate };