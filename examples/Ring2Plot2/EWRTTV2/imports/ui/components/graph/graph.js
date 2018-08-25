import './graph.html';

Template.graph.onCreated(function graphOnCreated() {
  // counter starts at 0
  this.counter = new ReactiveVar(0);
});

Template.graph.helpers({
  counter() {
    return Template.instance().counter.get();
  },
});

Template.graph.events({
  'click button'(event, instance) {
    // increment the counter when button is clicked
    instance.counter.set(instance.counter.get() + 1);
  },
});
