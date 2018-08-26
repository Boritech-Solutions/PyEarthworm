import './graph.html';
import { FlowRouter } from 'meteor/kadira:flow-router';
import { get_collection } from '/imports/startup/both/collections_manager.js';

Template.graph.onCreated(function graphOnCreated() {
  staname = FlowRouter.getParam('_id');
  Meteor.call('checkcol', {
    sta: staname }, 
    (err, res) => {
      if (err){
        alert(err);
        FlowRouter.go('App.home',{});
      } 
    }
  );
  console.log(staname);
  mystation = get_collection(staname);
});

Template.graph.helpers({
  counter() {
    console.log(mystation);
    return mystation.find().count();
  },
  station() {
    return staname;
  },
});

Template.graph.events({
  'click button'(event, instance) {
    // increment the counter when button is clicked
    instance.counter.set(instance.counter.get() + 1);
  },
});
