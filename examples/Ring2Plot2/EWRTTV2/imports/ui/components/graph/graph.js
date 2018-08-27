import './graph.html';
import { FlowRouter } from 'meteor/kadira:flow-router';
import { getCollection } from '/imports/api/collections_manager/collections_manager.js';
import { Meteor } from 'meteor/meteor';

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
  Meteor.subscribe(staname);
  mystation = getCollection(staname);

  for(var i = 0; i < mystation.find({}).count(); i++){

  }

});

Template.graph.helpers({
  counter() {
    return mystation.find({}).count();
  },
  station() {
    return staname;
  },
  channels (){
    console.log ('test');
    return mystation.find({});
  }
});

Template.graph.events({
  'click button'(event, instance) {
    // increment the counter when button is clicked
    instance.counter.set(instance.counter.get() + 1);
  },
});
