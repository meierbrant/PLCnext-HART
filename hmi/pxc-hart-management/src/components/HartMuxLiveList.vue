<template>
    <div id="hart-live-list">
        <h3><span data-bind="text: gateway"></span> Live List</h3>
        <table class="table table-striped">
            <thead>
                <tr>
                    <th>Long Tag</th>
                    <th>Name</th>
                    <th>Company</th>
                    <th class="d-none d-md-table-cell">Hart Revision</th>
                    <th>PV</th>
                    <th class="d-none d-md-table-cell">SV</th>
                    <th>Last Update</th>
                </tr>
            </thead>
            <tbody>
                <tr class="clickable" v-for="dev in devices" v-bind:key="10*dev.ioCard + dev.channel" v-on:click="navigateToDeviceShow(dev)">
                    <td>{{ dev.longTag }}</td>
                    <td>{{ dev.name }}</td>
                    <td>{{ dev.company }}</td>
                    <td class="d-none d-md-table-cell">{{ dev.cmdRevLvl }}</td>
                    <td>{{ displayVar(dev, 'pv') }}</td>
                    <td class="d-none d-md-table-cell">{{ displayVar(dev, 'sv') }}</td>
                    <td>{{ dev.vars ? dev.vars.loopCurrent.lastUpdated.toLocaleTimeString("en-US", {
                        hour: 'numeric',
                        minute: 'numeric',
                        day: 'numeric',
                        month: 'numeric'
                        }) : '' }}
                    </td>
                </tr>
            </tbody>
        </table>
    </div>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator';
import { HartMuxDto, hartServerUrl, HartGw, updateInterval } from '../types'
import { HttpResponse } from 'vue-resource/types/vue_resource';
import { HartVars, HartDeviceVarsListDto } from '../types/hart-vars';
import { HartDeviceDto } from '../types/hart-device';

const dummyVars: HartVars = {
    loopCurrent: {
        units: "mA",
        value: 2,
        lastUpdated: new Date()
    },
    pv: {
        units: "pi's",
        value: 3.141592,
        lastUpdated: new Date()
    },
    sv: {
        units: "<--",
        value: 2.71,
        lastUpdated: new Date()
    }
}

const dummyHartDevice = {
    name: "dummy device",
    company: "Phoenix Contact",
    longTag: "Dummy Long Tag",
    cmdRevLvl: 7,
    manufacturerId: 17,
    privateLabelDistCode: 0,
    profile: 1,
    revision: 1,
    vars: dummyVars
}

interface LiveListCache {
    gw: HartGw,
    devices: HartDeviceDto[]
}

type hartVarKey = 'loopCurrent' | 'pv' | 'sv' | 'tv' | 'qv'

@Component
export default class HartMuxLiveList extends Vue {
    @Prop() gwSN!: number
    public gw: HartGw = {ip: "", modules: [], serialNo: 0}
    public devices: HartDeviceDto[] = []
    private polling: number = 0
    private nextIoCardUpdate: number = 0

    mounted() {
        this.nextIoCardUpdate = 0
        this.polling = setInterval(this.refreshDevices, updateInterval)

        // pull live list from cache if present
        if (localStorage.liveList && localStorage.liveList.gw && localStorage.liveList.gw.serialNo) {
            const llc = JSON.parse(localStorage.liveList) as LiveListCache
            if (llc.gw.serialNo === this.gwSN) {
                this.gw = llc.gw
                llc.devices.forEach(dev => {
                    if (dev.vars) Object.keys(dev.vars).forEach(key => {
                        // FIXME
                        (dev.vars as any)[key]!.lastUpdated = new Date((dev.vars as any)[key]!.lastUpdated) // convert the date string back to a Date()
                    })
                })
                this.devices = llc.devices
            } else {
                this.gwLookup()
            }
        } else {
            this.gwLookup()
        }
    }

    beforeDestroy () {
        clearInterval(this.polling)

        // cache live list
        const llc: LiveListCache = {
            gw: this.gw,
            devices: this.devices
        }
        localStorage.liveList = JSON.stringify(llc)
    }

    public refreshDevices() {
        this.$http.get(hartServerUrl + '/gw/' + this.gwSN + '/vars/' + this.nextIoCardUpdate).then((res) => {
            const data = res.data as HartDeviceVarsListDto
            data.devices.forEach(updatedDevice => {
                const thisDevice = (dev: HartDeviceDto) => dev.ioCard == updatedDevice.ioCard && dev.channel == updatedDevice.channel
                const indexOD = this.devices.findIndex(thisDevice)
                Object.keys(updatedDevice.vars!).forEach(key => {
                    (updatedDevice.vars! as any)[key].lastUpdated = new Date()
                })
                if (indexOD >= 0) {
                    const oldDevice = this.devices[indexOD]
                    this.$set(this.devices, indexOD, updatedDevice)
                } else {
                    this.devices.push(updatedDevice)
                }
            })
        })
        if (this.gw.modules.length != 0) {
            this.nextIoCardUpdate = (this.nextIoCardUpdate + 1) % this.gw.modules.length;
        } else {
            this.gwLookup()
        }
    }

    public refreshDevicesNoVars(): PromiseLike<void | HttpResponse> {
        return this.$http.get(hartServerUrl + '/gw/' + this.gwSN + '/info').then((res) => {
            const data = res.data as HartDeviceVarsListDto
            let list: HartDeviceDto[] = []
            data.devices.forEach(device => {
                list.push(device)
            })
            this.devices = list
        })
    }

    public gwLookup () {
        this.$http.get(hartServerUrl + '/gw/discover').then(res => {
            const gws = res.data as HartGw[]
            this.gw = gws.find(gw => gw.serialNo === this.gwSN) || this.gw
        })
    }

    public displayVar(dev: HartDeviceDto, variable: hartVarKey): string {
        return (dev.vars && (dev.vars as any)[variable] && (dev.vars as any)[variable]!.value) ?
            ((dev.vars as any)[variable]!.value.toFixed(2) + "" + (dev.vars as any)[variable]!.units) : ''
    }

    public navigateToDeviceShow(dev: HartDeviceDto) {
        localStorage.device = JSON.stringify(dev)
        this.$router.push({ path: this.deviceUrl(dev) })
    }

    private deviceUrl(dev: HartDeviceDto): string {
        return '/gateways/' + this.gwSN + '/device/' + dev.ioCard + '/' + dev.channel
    }
}
</script>

<style lang="scss">

</style>