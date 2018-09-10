import { FlowRouter } from 'meteor/kadira:flow-router';

import './hello.html';

Template.hello.onCreated(function helloOnCreated() {
  // counter starts at 0
  this.counter = new ReactiveVar(0);
});

Template.hello.helpers({
  counter() {
    return Template.instance().counter.get();
  },
});

Template.hello.events({
  'submit .main-page-form-station' (event, instance) {
    event.preventDefault();
    var getStation = event.target.station.value;
    //console.log(getStation);
    Meteor.call('checkcol', { 
      sta: getStation
    }, (err, res) => {
      if (err){
        alert(err);
      } else {
        FlowRouter.go('Station.graph', { _id: getStation });
      }
    });
    
  },
});
